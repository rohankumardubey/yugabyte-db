// Copyright (c) YugaByte, Inc.
package com.yugabyte.yw.commissioner.tasks;

import com.yugabyte.yw.commissioner.BaseTaskDependencies;
import com.yugabyte.yw.commissioner.UserTaskDetails;
import com.yugabyte.yw.models.Universe;
import com.yugabyte.yw.models.XClusterConfig;
import com.yugabyte.yw.models.XClusterConfig.XClusterConfigStatusType;
import java.io.File;
import java.util.Optional;
import javax.inject.Inject;
import lombok.extern.slf4j.Slf4j;

@Slf4j
public class CreateXClusterConfig extends XClusterConfigTaskBase {

  @Inject
  protected CreateXClusterConfig(BaseTaskDependencies baseTaskDependencies) {
    super(baseTaskDependencies);
  }

  @Override
  public void run() {
    log.info("Running {}", getName());

    try {
      lockUniverseForUpdate(getUniverse().version);

      XClusterConfig xClusterConfig = getXClusterConfig();
      if (xClusterConfig.status != XClusterConfigStatusType.Init) {
        throw new RuntimeException(
            String.format(
                "XClusterConfig(%s) must be in `Init` state to create", xClusterConfig.uuid));
      }

      Universe sourceUniverse = Universe.getOrBadRequest(xClusterConfig.sourceUniverseUUID);
      Universe targetUniverse = Universe.getOrBadRequest(xClusterConfig.targetUniverseUUID);
      Optional<File> sourceCertificate =
          getSourceCertificateIfNecessary(sourceUniverse, targetUniverse);
      sourceCertificate.ifPresent(
          cert ->
              createSetupSourceCertificateTask(
                  targetUniverse, xClusterConfig.getReplicationGroupName(), cert));

      createXClusterConfigSetupTask()
          .setSubTaskGroupType(UserTaskDetails.SubTaskGroupType.ConfigureUniverse);
      createMarkUniverseUpdateSuccessTasks()
          .setSubTaskGroupType(UserTaskDetails.SubTaskGroupType.ConfigureUniverse);
      getRunnableTask().runSubTasks();

      setXClusterConfigStatus(XClusterConfigStatusType.Running);
    } catch (Exception e) {
      setXClusterConfigStatus(XClusterConfigStatusType.Failed);
      log.error("{} hit error : {}", getName(), e.getMessage());
      throw new RuntimeException(e);
    } finally {
      unlockUniverseForUpdate();
    }

    log.info("Completed {}", getName());
  }
}
