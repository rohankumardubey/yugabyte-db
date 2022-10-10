import React, { createContext, FC } from 'react';
import { UniverseConfigure, ClusterType, ClusterModes } from './utils/dto';
import { useFormMainStyles } from './universeMainStyle';
import { Box } from '@material-ui/core';
import { useMethods } from 'react-use';
import { RouteComponentProps } from 'react-router-dom';
import { CreateUniverse } from './CreateUniverse';
import { EditUniverse } from './EditUniverse';
import { EditReadReplica } from './EditRR';
import { CreateReadReplica } from './CreateRR';

interface UniverseFormContextState {
  clusterType: ClusterType;
  isPrimary?: boolean;
  UniverseConfigureData?: UniverseConfigure | null;
  FormData?: UniverseConfigure | null;
  mode: ClusterModes;
}

const initialState: UniverseFormContextState = {
  clusterType: ClusterType.PRIMARY,
  isPrimary: true,
  UniverseConfigureData: null,
  FormData: null,
  mode: ClusterModes.CREATE
};

const createFormMethods = (state: UniverseFormContextState) => ({
  setUniverseConfigureData: (data: UniverseConfigure): UniverseFormContextState => ({
    ...state,
    UniverseConfigureData: data
  }),
  toggleClusterType: (type: ClusterType) => ({
    ...state,
    clusterType: type === ClusterType.PRIMARY ? ClusterType.ASYNC : ClusterType.PRIMARY
  }),
  updateFormState: (payload: Partial<UniverseFormContextState>) => ({
    ...state,
    ...payload
  })
});

export const UniverseFormContext = createContext<any>(initialState);
export type FormContextMethods = ReturnType<typeof createFormMethods>;
interface UniverseFormContainerProps {
  mode: ClusterModes;
  pathname: string;
  uuid: string;
  clusterType: string;
}

export const UniverseFormContainer: FC<RouteComponentProps<{}, UniverseFormContainerProps>> = ({
  location,
  params
}) => {
  const classes = useFormMainStyles();
  const { clusterType, mode } = params;

  const universeContextData = useMethods(createFormMethods, initialState);

  return (
    <Box className={classes.mainConatiner}>
      <UniverseFormContext.Provider value={universeContextData}>
        {location.pathname === '/universe/new' && <CreateUniverse />}
        {mode === ClusterModes.EDIT && clusterType === ClusterType.PRIMARY && <EditUniverse />}
        {mode === ClusterModes.CREATE && clusterType === ClusterType.ASYNC && (
          <CreateReadReplica {...params} />
        )}
        {mode === ClusterModes.EDIT && clusterType === ClusterType.ASYNC && (
          <EditReadReplica {...params} />
        )}
      </UniverseFormContext.Provider>
    </Box>
  );
};
