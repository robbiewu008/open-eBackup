{{/*
Create the networks annotation for a deployment or statefulset
*/}}
{{- define "infrastructure.networksAnnotation" -}}
{{- $netPlane := "" }}
{{- $netList := "default/data" }}
{{- if .Values.global.InternalCommunicateNetPlane }}
{{- $netPlane = .Values.global.InternalCommunicateNetPlane | toString }}
{{- end }}
{{- if $netPlane }}
InternalCommunicateNetPlane: {{ $netPlane | quote }}
clusterUpdateTimeStamp: {{ .Values.global.clusterUpdateTimeStamp | toString | quote}}
{{- $netList = (printf "%s,default/%s" $netList $netPlane) }}
{{- end }}
k8s.v1.cni.cncf.io/networks: "{{ $netList }}"
{{- end }}


{{/*
Create the networks annotation for a deployment or statefulset
*/}}
{{- define "gaussdb.networksAnnotation" -}}
{{- $netPlane := "" }}
{{- $netList := "" }}
{{- if .Values.global.InternalCommunicateNetPlane }}
{{- $netPlane = .Values.global.InternalCommunicateNetPlane | toString }}
{{- end }}
{{- if $netPlane }}
InternalCommunicateNetPlane: {{ $netPlane | quote }}
clusterUpdateTimeStamp: {{ .Values.global.clusterUpdateTimeStamp | toString | quote}}
{{- $netList = (printf "default/%s" $netPlane) }}
k8s.v1.cni.cncf.io/networks: "{{ $netList }}"
{{- end }}
{{- end }}

{{/*
Create the container resource limits of frontend: internal communicate
*/}}
{{- define "limits.InternalCommunicateNetPlane" -}}
{{- $netplaneId := "" }}
{{- if .Values.global.InternalCommunicateNetPlane }}
{{- $netplaneId = .Values.global.InternalCommunicateNetPlane | toString }}
{{- end }}
{{- if $netplaneId }}
{{- $subPath:= (splitList ";" $netplaneId) }}
{{- $l_num:= (len $subPath) }}
{{- range $index := untilStep 0 $l_num 1}}
dorado/dev_vf_{{ ((index $subPath $index)) }}: 1
{{- end}}
{{- end }}
{{- end }}
