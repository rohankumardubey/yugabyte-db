// tslint:disable
/**
 * Yugabyte Cloud
 * YugabyteDB as a Service
 *
 * The version of the OpenAPI document: v1
 * Contact: support@yugabyte.com
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

// eslint-disable-next-line @typescript-eslint/ban-ts-comment
// @ts-ignore
import { useQuery, useInfiniteQuery, useMutation, UseQueryOptions, UseInfiniteQueryOptions, UseMutationOptions } from 'react-query';
import Axios from '../runtime';
import type { AxiosInstance } from 'axios';
// eslint-disable-next-line @typescript-eslint/ban-ts-comment
// @ts-ignore
import type {
  AddPlatformRequest,
  AddProjectToPlatformRequest,
  ApiError,
  PlatformListResponse,
  PlatformResponse,
  RefreshProviderPricingSpec,
} from '../models';

export interface AddPlatformForQuery {
  AddPlatformRequest?: AddPlatformRequest;
}
export interface AddProjectToPlatformForQuery {
  platformId: string;
  AddProjectToPlatformRequest?: AddProjectToPlatformRequest;
}
export interface GetPlatformForQuery {
  platformId: string;
}
export interface ListPlatformsForQuery {
  hostname?: string;
  cloud?: string;
  region?: string;
  under_maintenance?: boolean;
  order?: string;
  order_by?: string;
  limit?: number;
  continuation_token?: string;
}
export interface MarkPlatformsForMaintenanceForQuery {
  platformIds: Set<string>;
  maintenance_status: boolean;
}
export interface RefreshProviderPricingForQuery {
  RefreshProviderPricingSpec?: RefreshProviderPricingSpec;
}

/**
 * Add new platform
 * Add new platform
 */


export const addPlatformMutate = (
  body: AddPlatformForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  const url = '/private/platform';
  return Axios<PlatformResponse>(
    {
      url,
      method: 'POST',
      data: body.AddPlatformRequest
    },
    customAxiosInstance
  );
};

export const useAddPlatformMutation = <Error = ApiError>(
  options?: {
    mutation?:UseMutationOptions<PlatformResponse, Error>,
    customAxiosInstance?: AxiosInstance;
  }
) => {
  const {mutation: mutationOptions, customAxiosInstance} = options ?? {};
  // eslint-disable-next-line
  // @ts-ignore
  return useMutation<PlatformResponse, Error, AddPlatformForQuery, unknown>((props) => {
    return  addPlatformMutate(props, customAxiosInstance);
  }, mutationOptions);
};


/**
 * Assign project to platform
 * Add project to platform
 */


export const addProjectToPlatformMutate = (
  body: AddProjectToPlatformForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  const url = '/private/platform/{platformId}/project'.replace(`{${'platformId'}}`, encodeURIComponent(String(body.platformId)));
  // eslint-disable-next-line
  // @ts-ignore
  delete body.platformId;
  return Axios<unknown>(
    {
      url,
      method: 'POST',
      data: body.AddProjectToPlatformRequest
    },
    customAxiosInstance
  );
};

export const useAddProjectToPlatformMutation = <Error = ApiError>(
  options?: {
    mutation?:UseMutationOptions<unknown, Error>,
    customAxiosInstance?: AxiosInstance;
  }
) => {
  const {mutation: mutationOptions, customAxiosInstance} = options ?? {};
  // eslint-disable-next-line
  // @ts-ignore
  return useMutation<unknown, Error, AddProjectToPlatformForQuery, unknown>((props) => {
    return  addProjectToPlatformMutate(props, customAxiosInstance);
  }, mutationOptions);
};


/**
 * Get platform by ID
 * Get platform by ID
 */

export const getPlatformAxiosRequest = (
  requestParameters: GetPlatformForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<PlatformResponse>(
    {
      url: '/private/platform/{platformId}'.replace(`{${'platformId'}}`, encodeURIComponent(String(requestParameters.platformId))),
      method: 'GET',
      params: {
      }
    },
    customAxiosInstance
  );
};

export const getPlatformQueryKey = (
  requestParametersQuery: GetPlatformForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/private/platform/{platformId}`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useGetPlatformInfiniteQuery = <T = PlatformResponse, Error = ApiError>(
  params: GetPlatformForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<PlatformResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = getPlatformQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<PlatformResponse, Error, T>(
    queryKey,
    () => getPlatformAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useGetPlatformQuery = <T = PlatformResponse, Error = ApiError>(
  params: GetPlatformForQuery,
  options?: {
    query?: UseQueryOptions<PlatformResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = getPlatformQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<PlatformResponse, Error, T>(
    queryKey,
    () => getPlatformAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * List platforms
 * List platforms
 */

export const listPlatformsAxiosRequest = (
  requestParameters: ListPlatformsForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<PlatformListResponse>(
    {
      url: '/private/platform',
      method: 'GET',
      params: {
        hostname: requestParameters['hostname'],
        cloud: requestParameters['cloud'],
        region: requestParameters['region'],
        under_maintenance: requestParameters['under_maintenance'],
        order: requestParameters['order'],
        order_by: requestParameters['order_by'],
        limit: requestParameters['limit'],
        continuation_token: requestParameters['continuation_token'],
      }
    },
    customAxiosInstance
  );
};

export const listPlatformsQueryKey = (
  requestParametersQuery: ListPlatformsForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/private/platform`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useListPlatformsInfiniteQuery = <T = PlatformListResponse, Error = ApiError>(
  params: ListPlatformsForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<PlatformListResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = listPlatformsQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<PlatformListResponse, Error, T>(
    queryKey,
    () => listPlatformsAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useListPlatformsQuery = <T = PlatformListResponse, Error = ApiError>(
  params: ListPlatformsForQuery,
  options?: {
    query?: UseQueryOptions<PlatformListResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = listPlatformsQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<PlatformListResponse, Error, T>(
    queryKey,
    () => listPlatformsAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * Mark platforms as under maintenance
 * Mark Platforms for Maintenance
 */


export const markPlatformsForMaintenanceMutate = (
  customAxiosInstance?: AxiosInstance
) => {
  const url = '/private/platform/maintenance';
  return Axios<unknown>(
    {
      url,
      method: 'POST',
    },
    customAxiosInstance
  );
};

export const useMarkPlatformsForMaintenanceMutation = <Error = ApiError>(
  options?: {
    mutation?:UseMutationOptions<unknown, Error>,
    customAxiosInstance?: AxiosInstance;
  }
) => {
  const {mutation: mutationOptions, customAxiosInstance} = options ?? {};
  // eslint-disable-next-line
  // @ts-ignore
  return useMutation<unknown, Error, void, unknown>(() => {
    return  markPlatformsForMaintenanceMutate(customAxiosInstance);
  }, mutationOptions);
};


/**
 * Refresh pricing in specified existing customer providers
 * Refresh pricing in specified existing customer providers
 */


export const refreshProviderPricingMutate = (
  body: RefreshProviderPricingForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  const url = '/private/platform/providers/refresh-pricing';
  return Axios<unknown>(
    {
      url,
      method: 'PUT',
      data: body.RefreshProviderPricingSpec
    },
    customAxiosInstance
  );
};

export const useRefreshProviderPricingMutation = <Error = ApiError>(
  options?: {
    mutation?:UseMutationOptions<unknown, Error>,
    customAxiosInstance?: AxiosInstance;
  }
) => {
  const {mutation: mutationOptions, customAxiosInstance} = options ?? {};
  // eslint-disable-next-line
  // @ts-ignore
  return useMutation<unknown, Error, RefreshProviderPricingForQuery, unknown>((props) => {
    return  refreshProviderPricingMutate(props, customAxiosInstance);
  }, mutationOptions);
};





