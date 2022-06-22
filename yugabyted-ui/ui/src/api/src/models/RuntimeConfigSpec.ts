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




/**
 * Runtime Config value specification for a path
 * @export
 * @interface RuntimeConfigSpec
 */
export interface RuntimeConfigSpec  {
  /**
   * 
   * @type {string}
   * @memberof RuntimeConfigSpec
   */
  config_key: string;
  /**
   * 
   * @type {string}
   * @memberof RuntimeConfigSpec
   */
  config_value?: string;
  /**
   * 
   * @type {boolean}
   * @memberof RuntimeConfigSpec
   */
  override_entities?: boolean;
}



