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
import type { PlacementInfo } from './PlacementInfo';


/**
 * Cluster region info list
 * @export
 * @interface ClusterRegionInfo
 */
export interface ClusterRegionInfo  {
  /**
   * 
   * @type {PlacementInfo}
   * @memberof ClusterRegionInfo
   */
  placement_info: PlacementInfo;
  /**
   * 
   * @type {boolean}
   * @memberof ClusterRegionInfo
   */
  is_default?: boolean;
  /**
   * If the leaders should be pinned to this region
   * @type {boolean}
   * @memberof ClusterRegionInfo
   */
  is_affinitized?: boolean;
}



