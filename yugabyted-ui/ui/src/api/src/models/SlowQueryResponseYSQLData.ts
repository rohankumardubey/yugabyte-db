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
import type { SlowQueryResponseYSQLQueryItem } from './SlowQueryResponseYSQLQueryItem';


/**
 * Schema for Slow Query Response YSQL Data
 * @export
 * @interface SlowQueryResponseYSQLData
 */
export interface SlowQueryResponseYSQLData  {
  /**
   * Count of Errors
   * @type {number}
   * @memberof SlowQueryResponseYSQLData
   */
  error_count?: number;
  /**
   * 
   * @type {SlowQueryResponseYSQLQueryItem[]}
   * @memberof SlowQueryResponseYSQLData
   */
  queries?: SlowQueryResponseYSQLQueryItem[];
}



