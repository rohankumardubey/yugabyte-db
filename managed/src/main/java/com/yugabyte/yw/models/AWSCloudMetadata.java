package com.yugabyte.yw.models;

import java.util.HashMap;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

@Data
@JsonInclude(JsonInclude.Include.NON_NULL)
@JsonIgnoreProperties(ignoreUnknown = true)
public class AWSCloudMetadata implements CloudMetadata {

  @JsonIgnore
  final Map<String, String> configKeyMap =
      new HashMap<String, String>() {
        {
          put("awsAccessKeyID", "AWS_ACCESS_KEY_ID");
          put("awsAccessKeySecret", "AWS_SECRET_ACCESS_KEY");
          put("awsHostedZoneId", "HOSTED_ZONE_ID");
          put("awsHostedZoneName", "HOSTED_ZONE_NAME");
        }
      };

  // ToDo: Add Masking
  @JsonAlias("AWS_ACCESS_KEY_ID")
  @ApiModelProperty
  public String awsAccessKeyID;

  @JsonAlias("AWS_SECRET_ACCESS_KEY")
  @ApiModelProperty
  public String awsAccessKeySecret;

  @JsonAlias("HOSTED_ZONE_ID")
  @ApiModelProperty
  public String awsHostedZoneId;

  @JsonAlias("HOSTED_ZONE_NAME")
  @ApiModelProperty
  public String awsHostedZoneName;

  @JsonIgnore
  public Map<String, String> getEnvVars() {
    Map<String, String> envVars = new HashMap<>();

    if (awsAccessKeyID != null) {
      envVars.put("AWS_ACCESS_KEY_ID", awsAccessKeyID);
      envVars.put("AWS_SECRET_ACCESS_KEY", awsAccessKeySecret);
    }

    return envVars;
  }

  @JsonIgnore
  public Map<String, String> getConfigKeyMap() {
    return configKeyMap;
  }
}
