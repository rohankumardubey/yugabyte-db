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


// eslint-disable-next-line no-duplicate-imports
import type { InternalTagsSpec } from './InternalTagsSpec';
// eslint-disable-next-line no-duplicate-imports
import type { PagingMetadata } from './PagingMetadata';


/**
 * 
 * @export
 * @interface DefaultInternalTagsListResponse
 */
export interface DefaultInternalTagsListResponse  {
  /**
   * 
   * @type {InternalTagsSpec[]}
   * @memberof DefaultInternalTagsListResponse
   */
  data: InternalTagsSpec[];
  /**
   * 
   * @type {PagingMetadata}
   * @memberof DefaultInternalTagsListResponse
   */
  _metadata: PagingMetadata;
}



