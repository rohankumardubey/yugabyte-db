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
  ApiError,
  AppConfigResponse,
  LoginResponse,
  SSORedirectURLResponse,
  UserTutorialData,
  UserTutorialSpec,
  UserTutorialsResponse,
} from '../models';

export interface GetAppConfigForQuery {
  scopeType: GetAppConfigScopeTypeEnum;
  paths: Array<string>;
  scopeId?: string;
}
export interface GetSsoRedirectUrlForQuery {
  sso_type: GetSsoRedirectUrlSsoTypeEnum;
  sso_event_type: GetSsoRedirectUrlSsoEventTypeEnum;
}
export interface GetUserTutorialsForQuery {
  accountId: string;
  userId: string;
}
export interface SsoInviteCallbackForQuery {
  sso_type: SsoInviteCallbackSsoTypeEnum;
  code?: string;
  country_code?: string;
  marketing_consent?: boolean;
}
export interface SsoLoginCallbackForQuery {
  sso_type: SsoLoginCallbackSsoTypeEnum;
  code?: string;
}
export interface SsoSignupCallbackForQuery {
  sso_type: SsoSignupCallbackSsoTypeEnum;
  code?: string;
  country_code?: string;
  marketing_consent?: boolean;
}
export interface UpdateUserTutorialForQuery {
  accountId: string;
  userId: string;
  tutorialId: string;
  UserTutorialSpec?: UserTutorialSpec;
}
export interface UpdateUserTutorialEnabledForQuery {
  accountId: string;
  userId: string;
  tutorialId: string;
}
export interface UpdateUserTutorialStateForQuery {
  accountId: string;
  userId: string;
  tutorialId: string;
  stateId: string;
  is_completed: boolean;
}

/**
 * Get application configuration for a given set of paths
 * Get application configuration
 */

export const getAppConfigAxiosRequest = (
  requestParameters: GetAppConfigForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<AppConfigResponse>(
    {
      url: '/public/ui/app_config',
      method: 'GET',
      params: {
        scopeType: requestParameters['scopeType'],
        scopeId: requestParameters['scopeId'],
        paths: requestParameters['paths'],
      }
    },
    customAxiosInstance
  );
};

export const getAppConfigQueryKey = (
  requestParametersQuery: GetAppConfigForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/public/ui/app_config`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useGetAppConfigInfiniteQuery = <T = AppConfigResponse, Error = ApiError>(
  params: GetAppConfigForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<AppConfigResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = getAppConfigQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<AppConfigResponse, Error, T>(
    queryKey,
    () => getAppConfigAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useGetAppConfigQuery = <T = AppConfigResponse, Error = ApiError>(
  params: GetAppConfigForQuery,
  options?: {
    query?: UseQueryOptions<AppConfigResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = getAppConfigQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<AppConfigResponse, Error, T>(
    queryKey,
    () => getAppConfigAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * Retrieve redirect URL for Single Sign On using external authentication.
 * Retrieve redirect URL for Single Sign On using external authentication.
 */

export const getSsoRedirectUrlAxiosRequest = (
  requestParameters: GetSsoRedirectUrlForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<SSORedirectURLResponse>(
    {
      url: '/public/ui/sso_redirect_url',
      method: 'GET',
      params: {
        sso_type: requestParameters['sso_type'],
        sso_event_type: requestParameters['sso_event_type'],
      }
    },
    customAxiosInstance
  );
};

export const getSsoRedirectUrlQueryKey = (
  requestParametersQuery: GetSsoRedirectUrlForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/public/ui/sso_redirect_url`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useGetSsoRedirectUrlInfiniteQuery = <T = SSORedirectURLResponse, Error = ApiError>(
  params: GetSsoRedirectUrlForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<SSORedirectURLResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = getSsoRedirectUrlQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<SSORedirectURLResponse, Error, T>(
    queryKey,
    () => getSsoRedirectUrlAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useGetSsoRedirectUrlQuery = <T = SSORedirectURLResponse, Error = ApiError>(
  params: GetSsoRedirectUrlForQuery,
  options?: {
    query?: UseQueryOptions<SSORedirectURLResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = getSsoRedirectUrlQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<SSORedirectURLResponse, Error, T>(
    queryKey,
    () => getSsoRedirectUrlAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * Get tutorials for a user
 * Get tutorials for a user
 */

export const getUserTutorialsAxiosRequest = (
  requestParameters: GetUserTutorialsForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<UserTutorialsResponse>(
    {
      url: '/public/ui/accounts/{accountId}/users/{userId}/tutorials'.replace(`{${'accountId'}}`, encodeURIComponent(String(requestParameters.accountId))).replace(`{${'userId'}}`, encodeURIComponent(String(requestParameters.userId))),
      method: 'GET',
      params: {
      }
    },
    customAxiosInstance
  );
};

export const getUserTutorialsQueryKey = (
  requestParametersQuery: GetUserTutorialsForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/public/ui/accounts/{accountId}/users/{userId}/tutorials`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useGetUserTutorialsInfiniteQuery = <T = UserTutorialsResponse, Error = ApiError>(
  params: GetUserTutorialsForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<UserTutorialsResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = getUserTutorialsQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<UserTutorialsResponse, Error, T>(
    queryKey,
    () => getUserTutorialsAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useGetUserTutorialsQuery = <T = UserTutorialsResponse, Error = ApiError>(
  params: GetUserTutorialsForQuery,
  options?: {
    query?: UseQueryOptions<UserTutorialsResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = getUserTutorialsQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<UserTutorialsResponse, Error, T>(
    queryKey,
    () => getUserTutorialsAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * Callback for SSO invite
 * Callback for SSO invite
 */

export const ssoInviteCallbackAxiosRequest = (
  requestParameters: SsoInviteCallbackForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<LoginResponse>(
    {
      url: '/public/ui/callback/sso_invite',
      method: 'GET',
      params: {
        sso_type: requestParameters['sso_type'],
        code: requestParameters['code'],
        country_code: requestParameters['country_code'],
        marketing_consent: requestParameters['marketing_consent'],
      }
    },
    customAxiosInstance
  );
};

export const ssoInviteCallbackQueryKey = (
  requestParametersQuery: SsoInviteCallbackForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/public/ui/callback/sso_invite`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useSsoInviteCallbackInfiniteQuery = <T = LoginResponse, Error = ApiError>(
  params: SsoInviteCallbackForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<LoginResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = ssoInviteCallbackQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<LoginResponse, Error, T>(
    queryKey,
    () => ssoInviteCallbackAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useSsoInviteCallbackQuery = <T = LoginResponse, Error = ApiError>(
  params: SsoInviteCallbackForQuery,
  options?: {
    query?: UseQueryOptions<LoginResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = ssoInviteCallbackQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<LoginResponse, Error, T>(
    queryKey,
    () => ssoInviteCallbackAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * Callback for SSO login
 * Callback for SSO login
 */

export const ssoLoginCallbackAxiosRequest = (
  requestParameters: SsoLoginCallbackForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<LoginResponse>(
    {
      url: '/public/ui/callback/sso_login',
      method: 'GET',
      params: {
        sso_type: requestParameters['sso_type'],
        code: requestParameters['code'],
      }
    },
    customAxiosInstance
  );
};

export const ssoLoginCallbackQueryKey = (
  requestParametersQuery: SsoLoginCallbackForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/public/ui/callback/sso_login`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useSsoLoginCallbackInfiniteQuery = <T = LoginResponse, Error = ApiError>(
  params: SsoLoginCallbackForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<LoginResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = ssoLoginCallbackQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<LoginResponse, Error, T>(
    queryKey,
    () => ssoLoginCallbackAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useSsoLoginCallbackQuery = <T = LoginResponse, Error = ApiError>(
  params: SsoLoginCallbackForQuery,
  options?: {
    query?: UseQueryOptions<LoginResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = ssoLoginCallbackQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<LoginResponse, Error, T>(
    queryKey,
    () => ssoLoginCallbackAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * Callback for SSO signup
 * Callback for SSO signup
 */

export const ssoSignupCallbackAxiosRequest = (
  requestParameters: SsoSignupCallbackForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  return Axios<LoginResponse>(
    {
      url: '/public/ui/callback/sso_signup',
      method: 'GET',
      params: {
        sso_type: requestParameters['sso_type'],
        code: requestParameters['code'],
        country_code: requestParameters['country_code'],
        marketing_consent: requestParameters['marketing_consent'],
      }
    },
    customAxiosInstance
  );
};

export const ssoSignupCallbackQueryKey = (
  requestParametersQuery: SsoSignupCallbackForQuery,
  pageParam = -1,
  version = 1,
) => [
  `/v${version}/public/ui/callback/sso_signup`,
  pageParam,
  ...(requestParametersQuery ? [requestParametersQuery] : [])
];


export const useSsoSignupCallbackInfiniteQuery = <T = LoginResponse, Error = ApiError>(
  params: SsoSignupCallbackForQuery,
  options?: {
    query?: UseInfiniteQueryOptions<LoginResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  pageParam = -1,
  version = 1,
) => {
  const queryKey = ssoSignupCallbackQueryKey(params, pageParam, version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useInfiniteQuery<LoginResponse, Error, T>(
    queryKey,
    () => ssoSignupCallbackAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};

export const useSsoSignupCallbackQuery = <T = LoginResponse, Error = ApiError>(
  params: SsoSignupCallbackForQuery,
  options?: {
    query?: UseQueryOptions<LoginResponse, Error, T>;
    customAxiosInstance?: AxiosInstance;
  },
  version = 1,
) => {
  const queryKey = ssoSignupCallbackQueryKey(params,  version);
  const { query: queryOptions, customAxiosInstance } = options ?? {};

  const query = useQuery<LoginResponse, Error, T>(
    queryKey,
    () => ssoSignupCallbackAxiosRequest(params, customAxiosInstance),
    queryOptions
  );

  return {
    queryKey,
    ...query
  };
};



/**
 * Update tutorial for a user
 * Update tutorial for a user
 */


export const updateUserTutorialMutate = (
  body: UpdateUserTutorialForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  const url = '/public/ui/accounts/{accountId}/users/{userId}/tutorials/{tutorialId}'.replace(`{${'accountId'}}`, encodeURIComponent(String(body.accountId))).replace(`{${'userId'}}`, encodeURIComponent(String(body.userId))).replace(`{${'tutorialId'}}`, encodeURIComponent(String(body.tutorialId)));
  // eslint-disable-next-line
  // @ts-ignore
  delete body.accountId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.userId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.tutorialId;
  return Axios<UserTutorialData>(
    {
      url,
      method: 'PUT',
      data: body.UserTutorialSpec
    },
    customAxiosInstance
  );
};

export const useUpdateUserTutorialMutation = <Error = ApiError>(
  options?: {
    mutation?:UseMutationOptions<UserTutorialData, Error>,
    customAxiosInstance?: AxiosInstance;
  }
) => {
  const {mutation: mutationOptions, customAxiosInstance} = options ?? {};
  // eslint-disable-next-line
  // @ts-ignore
  return useMutation<UserTutorialData, Error, UpdateUserTutorialForQuery, unknown>((props) => {
    return  updateUserTutorialMutate(props, customAxiosInstance);
  }, mutationOptions);
};


/**
 * Update whether tutorial is enabled for a user
 * Update whether tutorial is enabled for a user
 */


export const updateUserTutorialEnabledMutate = (
  body: UpdateUserTutorialEnabledForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  const url = '/public/ui/accounts/{accountId}/users/{userId}/tutorials/{tutorialId}/enabled'.replace(`{${'accountId'}}`, encodeURIComponent(String(body.accountId))).replace(`{${'userId'}}`, encodeURIComponent(String(body.userId))).replace(`{${'tutorialId'}}`, encodeURIComponent(String(body.tutorialId)));
  // eslint-disable-next-line
  // @ts-ignore
  delete body.accountId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.userId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.tutorialId;
  return Axios<unknown>(
    {
      url,
      method: 'PUT',
    },
    customAxiosInstance
  );
};

export const useUpdateUserTutorialEnabledMutation = <Error = ApiError>(
  options?: {
    mutation?:UseMutationOptions<unknown, Error>,
    customAxiosInstance?: AxiosInstance;
  }
) => {
  const {mutation: mutationOptions, customAxiosInstance} = options ?? {};
  // eslint-disable-next-line
  // @ts-ignore
  return useMutation<unknown, Error, UpdateUserTutorialEnabledForQuery, unknown>((props) => {
    return  updateUserTutorialEnabledMutate(props, customAxiosInstance);
  }, mutationOptions);
};


/**
 * Update tutorial state status for a user
 * Update tutorial state status for a user
 */


export const updateUserTutorialStateMutate = (
  body: UpdateUserTutorialStateForQuery,
  customAxiosInstance?: AxiosInstance
) => {
  const url = '/public/ui/accounts/{accountId}/users/{userId}/tutorials/{tutorialId}/state/{stateId}/{is_completed}'.replace(`{${'accountId'}}`, encodeURIComponent(String(body.accountId))).replace(`{${'userId'}}`, encodeURIComponent(String(body.userId))).replace(`{${'tutorialId'}}`, encodeURIComponent(String(body.tutorialId))).replace(`{${'stateId'}}`, encodeURIComponent(String(body.stateId))).replace(`{${'is_completed'}}`, encodeURIComponent(String(body.is_completed)));
  // eslint-disable-next-line
  // @ts-ignore
  delete body.accountId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.userId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.tutorialId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.stateId;
  // eslint-disable-next-line
  // @ts-ignore
  delete body.is_completed;
  return Axios<UserTutorialData>(
    {
      url,
      method: 'PUT',
    },
    customAxiosInstance
  );
};

export const useUpdateUserTutorialStateMutation = <Error = ApiError>(
  options?: {
    mutation?:UseMutationOptions<UserTutorialData, Error>,
    customAxiosInstance?: AxiosInstance;
  }
) => {
  const {mutation: mutationOptions, customAxiosInstance} = options ?? {};
  // eslint-disable-next-line
  // @ts-ignore
  return useMutation<UserTutorialData, Error, UpdateUserTutorialStateForQuery, unknown>((props) => {
    return  updateUserTutorialStateMutate(props, customAxiosInstance);
  }, mutationOptions);
};






/**
  * @export
  * @enum {string}
  */
export enum GetAppConfigScopeTypeEnum {
  Global = 'GLOBAL',
  Account = 'ACCOUNT'
}
/**
  * @export
  * @enum {string}
  */
export enum GetSsoRedirectUrlSsoTypeEnum {
  Google = 'GOOGLE',
  Github = 'GITHUB',
  Linkedin = 'LINKEDIN'
}
/**
  * @export
  * @enum {string}
  */
export enum GetSsoRedirectUrlSsoEventTypeEnum {
  Signup = 'SIGNUP',
  Login = 'LOGIN',
  Invite = 'INVITE'
}
/**
  * @export
  * @enum {string}
  */
export enum SsoInviteCallbackSsoTypeEnum {
  Google = 'GOOGLE',
  Github = 'GITHUB',
  Linkedin = 'LINKEDIN'
}
/**
  * @export
  * @enum {string}
  */
export enum SsoLoginCallbackSsoTypeEnum {
  Google = 'GOOGLE',
  Github = 'GITHUB',
  Linkedin = 'LINKEDIN'
}
/**
  * @export
  * @enum {string}
  */
export enum SsoSignupCallbackSsoTypeEnum {
  Google = 'GOOGLE',
  Github = 'GITHUB',
  Linkedin = 'LINKEDIN'
}
