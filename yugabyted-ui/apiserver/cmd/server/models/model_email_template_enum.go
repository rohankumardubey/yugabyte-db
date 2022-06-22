package models
// EmailTemplateEnum : Which type of email to send
type EmailTemplateEnum string

// List of EmailTemplateEnum
const (
	EMAILTEMPLATEENUM_USER_ONBOARDING EmailTemplateEnum = "USER_ONBOARDING"
	EMAILTEMPLATEENUM_SALES_USER_SIGNUP EmailTemplateEnum = "SALES_USER_SIGNUP"
	EMAILTEMPLATEENUM_ABANDONED_CLUSTER_PAUSED EmailTemplateEnum = "ABANDONED_CLUSTER_PAUSED"
	EMAILTEMPLATEENUM_ABANDONED_CLUSTER_WARNING EmailTemplateEnum = "ABANDONED_CLUSTER_WARNING"
	EMAILTEMPLATEENUM_ABANDONED_CLUSTER_DELETED EmailTemplateEnum = "ABANDONED_CLUSTER_DELETED"
	EMAILTEMPLATEENUM_ALERT_CLUSTER_DISK_SPACE_LOW EmailTemplateEnum = "ALERT_CLUSTER_DISK_SPACE_LOW"
	EMAILTEMPLATEENUM_ALERT_CLUSTER EmailTemplateEnum = "ALERT_CLUSTER"
	EMAILTEMPLATEENUM_ALERT_BILLING EmailTemplateEnum = "ALERT_BILLING"
	EMAILTEMPLATEENUM_ALERT_CLUSTER_TEST EmailTemplateEnum = "ALERT_CLUSTER_TEST"
	EMAILTEMPLATEENUM_CLUSTER_CREATED EmailTemplateEnum = "CLUSTER_CREATED"
	EMAILTEMPLATEENUM_PAUSED_CLUSTER_NOTIFICATION EmailTemplateEnum = "PAUSED_CLUSTER_NOTIFICATION"
	EMAILTEMPLATEENUM_UPGRADE_SCHEDULED_FREE EmailTemplateEnum = "UPGRADE_SCHEDULED_FREE"
	EMAILTEMPLATEENUM_UPGRADE_SCHEDULED_RF1 EmailTemplateEnum = "UPGRADE_SCHEDULED_RF1"
	EMAILTEMPLATEENUM_UPGRADE_SCHEDULED EmailTemplateEnum = "UPGRADE_SCHEDULED"
	EMAILTEMPLATEENUM_MAINTENANCE_EVENT EmailTemplateEnum = "MAINTENANCE_EVENT"
	EMAILTEMPLATEENUM_MAINTENANCE_EVENT_COMPLETION_SUCCESS EmailTemplateEnum = "MAINTENANCE_EVENT_COMPLETION_SUCCESS"
	EMAILTEMPLATEENUM_MAINTENANCE_EVENT_COMPLETION_FAILURE EmailTemplateEnum = "MAINTENANCE_EVENT_COMPLETION_FAILURE"
	EMAILTEMPLATEENUM_CUSTOM EmailTemplateEnum = "CUSTOM"
	EMAILTEMPLATEENUM_SINGLE_CREDIT_THRESHOLD_ALERT EmailTemplateEnum = "SINGLE_CREDIT_THRESHOLD_ALERT"
	EMAILTEMPLATEENUM_TOTAL_CREDIT_THRESHOLD_ALERT EmailTemplateEnum = "TOTAL_CREDIT_THRESHOLD_ALERT"
)
