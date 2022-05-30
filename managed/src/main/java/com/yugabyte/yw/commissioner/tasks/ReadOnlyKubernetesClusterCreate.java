/*
 * Copyright 2022 YugaByte, Inc. and Contributors
 *
 * Licensed under the Polyform Free Trial License 1.0.0 (the "License"); you
 * may not use this file except in compliance with the License. You
 * may obtain a copy of the License at
 *
 * http://github.com/YugaByte/yugabyte-db/blob/master/licenses/POLYFORM-FREE-TRIAL-LICENSE-1.0.0.txt
 */

package com.yugabyte.yw.commissioner.tasks;

import com.yugabyte.yw.commissioner.BaseTaskDependencies;
import com.yugabyte.yw.commissioner.Common.CloudType;
import com.yugabyte.yw.commissioner.tasks.subtasks.KubernetesCommandExecutor;
import com.yugabyte.yw.commissioner.UserTaskDetails.SubTaskGroupType;
import com.yugabyte.yw.common.PlacementInfoUtil;
import com.yugabyte.yw.forms.UniverseDefinitionTaskParams.Cluster;
import com.yugabyte.yw.models.helpers.NodeDetails;
import com.yugabyte.yw.models.helpers.PlacementInfo;
import com.yugabyte.yw.models.Provider;
import com.yugabyte.yw.models.Universe;
import java.util.Set;
import java.util.UUID;
import javax.inject.Inject;
import lombok.extern.slf4j.Slf4j;

// Tracks the read only kubernetes cluster create intent within an existing universe.
@Slf4j
public class ReadOnlyKubernetesClusterCreate extends KubernetesTaskBase {
  @Inject
  protected ReadOnlyKubernetesClusterCreate(BaseTaskDependencies baseTaskDependencies) {
    super(baseTaskDependencies);
  }

  @Override
  public void run() {
    log.info("Started {} task for uuid={}", getName(), taskParams().universeUUID);
    try {
      Universe universe = lockUniverseForUpdate(taskParams().expectedUniverseVersion);
      preTaskActions(universe);

      // Set all the in-memory node names first.
      setNodeNames(universe);

      Cluster primaryCluster = universe.getUniverseDetails().getPrimaryCluster();

      universe = writeUserIntentToUniverse(true);

      Cluster readOnlyCluster = taskParams().getReadOnlyClusters().get(0);
      PlacementInfo pi = readOnlyCluster.placementInfo;

      Provider primaryProvider =
          Provider.getOrBadRequest(UUID.fromString(primaryCluster.userIntent.provider));
      Provider provider =
          Provider.getOrBadRequest(UUID.fromString(readOnlyCluster.userIntent.provider));

      KubernetesPlacement placement = new KubernetesPlacement(pi);

      CloudType primaryCloudType = primaryCluster.userIntent.providerType;
      if (primaryCloudType != CloudType.kubernetes) {
        String msg =
            String.format(
                "Expected primary cluster on kubernetes but found on %s", primaryCloudType.name());
        log.error(msg);
        throw new IllegalArgumentException(msg);
      }

      PlacementInfo primaryPI = primaryCluster.placementInfo;
      KubernetesPlacement primaryPlacement = new KubernetesPlacement(primaryPI);

      boolean newNamingStyle = taskParams().useNewHelmNamingStyle;

      String masterAddresses =
          PlacementInfoUtil.computeMasterAddresses(
              primaryPI,
              primaryPlacement.masters,
              taskParams().nodePrefix,
              primaryProvider,
              taskParams().communicationPorts.masterRpcPort,
              newNamingStyle);

      boolean isMultiAz = PlacementInfoUtil.isMultiAZ(provider);
      createPodsTask(placement, masterAddresses, true);

      // Following method assumes primary cluster.
      createSingleKubernetesExecutorTask(KubernetesCommandExecutor.CommandType.POD_INFO, pi, true);

      Set<NodeDetails> tserversAdded =
          getPodsToAdd(placement.tservers, null, ServerType.TSERVER, isMultiAz, true);

      // Wait for new tablet servers to be responsive.
      createWaitForServersTasks(tserversAdded, ServerType.TSERVER)
          .setSubTaskGroupType(SubTaskGroupType.ConfigureUniverse);

      // Persist the placement info into the YB master leader.
      createPlacementInfoTask(null /* blacklistNodes */)
          .setSubTaskGroupType(SubTaskGroupType.ConfigureUniverse);

      // Wait for a master leader to hear from all the tservers.
      createWaitForTServerHeartBeatsTask().setSubTaskGroupType(SubTaskGroupType.ConfigureUniverse);

      createSwamperTargetUpdateTask(false);
      // Marks the update of this universe as a success only if all the tasks before it succeeded.
      createMarkUniverseUpdateSuccessTasks()
          .setSubTaskGroupType(SubTaskGroupType.ConfigureUniverse);

      // Run all the tasks.
      getRunnableTask().runSubTasks();
    } catch (Throwable t) {
      log.error("Error executing task {}, error='{}'", getName(), t.getMessage(), t);
      throw t;
    } finally {
      unlockUniverseForUpdate();
    }
    log.info("Finished {} task.", getName());
  }
}
