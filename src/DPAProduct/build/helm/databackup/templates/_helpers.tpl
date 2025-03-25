{{/*
Expand the name of the chart.
*/}}
{{- define "oceanprotect-dataprotect.name" -}}
{{- default .Chart.Name .Values.nameOverride | trunc 63 | trimSuffix "-" }}
{{- end }}

{{/*
Create a default fully qualified app name.
We truncate at 63 chars because some Kubernetes name fields are limited to this (by the DNS naming spec).
If release name contains chart name it will be used as a full name.
*/}}
{{- define "oceanprotect-dataprotect.fullname" -}}
{{- if .Values.fullnameOverride }}
{{- .Values.fullnameOverride | trunc 63 | trimSuffix "-" }}
{{- else }}
{{- $name := default .Chart.Name .Values.nameOverride }}
{{- if contains $name .Release.Name }}
{{- .Release.Name | trunc 63 | trimSuffix "-" }}
{{- else }}
{{- printf "%s-%s" .Release.Name $name | trunc 63 | trimSuffix "-" }}
{{- end }}
{{- end }}
{{- end }}

{{/*
Create chart name and version as used by the chart label.
*/}}
{{- define "oceanprotect-dataprotect.chart" -}}
{{- printf "%s-%s" .Chart.Name .Chart.Version | replace "+" "_" | trunc 63 | trimSuffix "-" }}
{{- end }}

{{/*
Common labels
*/}}
{{- define "oceanprotect-dataprotect.labels" -}}
helm.sh/chart: {{ include "oceanprotect-dataprotect.chart" . }}
{{ include "oceanprotect-dataprotect.selectorLabels" . }}
{{- if .Chart.AppVersion }}
app.kubernetes.io/version: {{ .Chart.AppVersion | quote }}
{{- end }}
app.kubernetes.io/managed-by: {{ .Release.Service }}
{{- end }}

{{/*
Selector labels
*/}}
{{- define "oceanprotect-dataprotect.selectorLabels" -}}
app.kubernetes.io/name: {{ include "oceanprotect-dataprotect.name" . }}
app.kubernetes.io/instance: {{ .Release.Name }}
{{- end }}


{{/*
获取部署型号
*/}}
{{- define "getDeployType" -}}
{{- $deploytype:= .Values.global.deploy_type | default "" }}
{{- if or (eq $deploytype "d0") (eq $deploytype "d1") (eq $deploytype "d2") (eq $deploytype "d3") (eq $deploytype "d4") (eq $deploytype "d5")  (eq $deploytype "d6") (eq $deploytype "d7") (eq $deploytype "d8") (eq $deploytype "d9") (eq $deploytype "d10")}}
{{- $deploytype }}
{{- else if and (eq $deploytype "dataprotect") (has .Values.global.productModel .Values.global.support_board_type.x8000) }}
{{- $true_deploy_type := "d0" }}
{{- $true_deploy_type }}
{{- else if and (eq $deploytype "dataprotect") (has .Values.global.productModel .Values.global.support_board_type.x8000k) }}
{{- $true_deploy_type := "d0" }}
{{- $true_deploy_type }}
{{- else if and (eq $deploytype "dataprotect") (has .Values.global.productModel .Values.global.support_board_type.x6000) }}
{{- $true_deploy_type := "d1" }}
{{- $true_deploy_type }}
{{- else if and (eq $deploytype "dataprotect") (has .Values.global.productModel .Values.global.support_board_type.x3000) }}
{{- $true_deploy_type := "d2" }}
{{- $true_deploy_type }}
{{- else if and (eq $deploytype "cloudbackup") (has .Values.global.productModel .Values.global.support_board_type.cloudbackup) }}
{{- $true_deploy_type := "d3" }}
{{- $true_deploy_type }}
{{- else if and (eq $deploytype "ransomware") (has .Values.global.productModel .Values.global.support_board_type.ransomware) }}
{{- $true_deploy_type := "d4" }}
{{- $true_deploy_type }}
{{- else if and (eq $deploytype "dataprotect") (has .Values.global.productModel .Values.global.support_board_type.x9000) }}
{{- $true_deploy_type := "d6" }}
{{- $true_deploy_type }}
{{- else if and (eq $deploytype "dataprotect") (has .Values.global.productModel .Values.global.support_board_type.x9000k) }}
{{- $true_deploy_type := "d6" }}
{{- $true_deploy_type }}
{{- else }}
{{- fail "productModel not support" }}
{{- end }}
{{- end }}


