package com.yugabyte.yw.models;

import java.util.HashMap;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import io.swagger.annotations.ApiModelProperty;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;

@Slf4j
@Data
public class OnPremCloudMetadata implements CloudMetadata {

  @JsonProperty("YB_HOME_DIR")
  @ApiModelProperty
  public String ybHomeDir;

  @JsonIgnore
  public Map<String, String> getEnvVars() {
    Map<String, String> envVars = new HashMap<>();

    envVars.put("YB_HOME_DIR", this.ybHomeDir);
    return envVars;
  }

  @JsonIgnore
  public void updateCloudMetadataDetails(String key, String value) {
    // pass
  }
}
