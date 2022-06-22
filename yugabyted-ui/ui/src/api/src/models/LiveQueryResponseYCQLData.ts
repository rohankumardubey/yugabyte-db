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
import type { LiveQueryResponseYCQLQueryItem } from './LiveQueryResponseYCQLQueryItem';


/**
 * Schema for Live Query Response YCQL Data
 * @export
 * @interface LiveQueryResponseYCQLData
 */
export interface LiveQueryResponseYCQLData  {
  /**
   * Count of Errors
   * @type {number}
   * @memberof LiveQueryResponseYCQLData
   */
  error_count?: number;
  /**
   * 
   * @type {LiveQueryResponseYCQLQueryItem[]}
   * @memberof LiveQueryResponseYCQLData
   */
  queries?: LiveQueryResponseYCQLQueryItem[];
}



