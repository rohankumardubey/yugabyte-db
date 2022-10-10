import React, { FC, useContext } from 'react';
import { useQuery } from 'react-query';
import { useTranslation } from 'react-i18next';
import { UniverseForm } from './UniverseForm';
import { ClusterType, DEFAULT_FORM_DATA, UniverseFormData, ClusterModes } from './utils/dto';
import { UniverseFormContext } from './UniverseFormContainer';
import { api, QUERY_KEY } from './utils/api';

interface EditReadReplicaProps {
  uuid: string;
}

export const EditReadReplica: FC<EditReadReplicaProps> = (props) => {
  const { t } = useTranslation();
  const [state, formMethods] = useContext(UniverseFormContext);
  const { uuid } = props;

  const { isLoading, data: universe } = useQuery(
    [QUERY_KEY.fetchUniverse, uuid],
    () => api.fetchUniverse(uuid),
    {
      onSuccess: () => {
        formMethods.updateFormState({
          clusterType: ClusterType.ASYNC,
          mode: ClusterModes.EDIT,
          isPrimary: false
        });
      }
    }
  );

  if (isLoading || !state) return <>Loading .... </>;

  const onSubmit = (formData: UniverseFormData) => {
    console.log(formData);
  };

  return (
    <UniverseForm
      defaultFormData={DEFAULT_FORM_DATA}
      title={
        <>
          {universe?.name}
          <span>
            {' '}
            <i className="fa fa-chevron-right"></i> {t('universeForm.configReadReplica')}{' '}
          </span>
        </>
      }
      onFormSubmit={(data: UniverseFormData) => onSubmit(data)}
      onCancel={() => console.log('cancelled')}
    />
  );
};
