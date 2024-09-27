{{/*
Create the networks annotation for a deployment or statefulset
*/}}
{{- define "data-enable-engine.networksAnnotation" -}}
{{- $backup := "" }}
{{- $archive := "" }}
{{- $netList := "default/data" }}
{{- $backup = .Values.global.backupNetPlane | toString }}
{{- $archive = .Values.global.archiveNetPlane | toString }}

{{- if and $backup $archive }}
{{- $subPath:= (splitList ";" $backup) }}
{{- $l_num:= (len $subPath) }}
{{- range $index := untilStep 0 $l_num 1}}
{{- if eq $index 0}}
backupNetPlane: {{ index $subPath $index | quote }}
{{- else}}
backupNetPlane_{{ $index }}: {{ index $subPath $index | quote }}
{{- end}}
  {{- $netList = (printf "%s,default/%s" $netList ((index $subPath $index))) }}
{{- end }}

archiveNetPlane: {{ $archive | quote }}
  {{- $netList = (printf "%s,default/%s" $netList $archive) }}
{{- else if $backup }}
{{- $subPath:= (splitList ";" $backup) }}
{{- $l_num:= (len $subPath) }}
{{- range $index := untilStep 0 $l_num 1}}
{{- if eq $index 0}}
backupNetPlane: {{ index $subPath $index | quote }}
{{- else}}
backupNetPlane_{{ $index }}: {{ index $subPath $index | quote }}
{{- end}}
  {{- $netList = (printf "%s,default/%s" $netList ((index $subPath $index))) }}
{{- end }}
{{- else if $archive }}
  {{- $netList = (printf "%s,default/%s" $netList $archive) }}
archiveNetPlane: {{ $archive | quote }}
{{- end}}
k8s.v1.cni.cncf.io/networks: "{{ $netList }}"
{{- end }}

{{/*
Create the updateTimestamp annotation for the pod template in a deployment or statefulset
*/}}
{{- define "data-enable-engine.updateTimestampPodAnnotation" -}}
{{- $updateTimestamp := "" }}
{{- $backup := "" }}
{{- $archive := "" }}


{{- $backup := .Values.global.backupNetPlane | toString }}
{{- $archive := .Values.global.archiveNetPlane | toString }}
{{- range $key, $val := .Values.global.updateTimestamp }}

{{- if eq $backup  $key }}
backupUpdateTimestamp: "{{ $val }}"
{{- end -}}
{{- if eq $archive  $key }}
archiveUpdateTimestamp: "{{ $val }}"
{{- end -}}
{{- end }}
{{- end }}


{{/*
Create the networks annotation for a deployment or statefulset
*/}}
{{- define "protectengine-a.networksAnnotation" -}}
{{- $backup := "" }}
{{- $copy := "" }}
{{- $storage := "" }}
{{- $netList := "default/data" }}
{{- if .Values.global }}
{{- $backup = .Values.global.backupNetPlane | toString }}
{{- end }}
{{- if .Values.global.copyNetPlane }}
{{- $copy = .Values.global.copyNetPlane | toString }}
{{- end }}
{{- if .Values.global.storageNetPlane }}
{{- $storage = .Values.global.storageNetPlane | toString }}
{{- end }}
{{- if and $backup $copy }}
{{- $subPath:= (splitList ";" $backup) }}
{{- $l_num:= (len $subPath) }}
{{- range $index := untilStep 0 $l_num 1}}
{{- if eq $index 0}}
backupNetPlane: {{ index $subPath $index | quote }}
{{- else}}
backupNetPlane_{{ $index }}: {{ index $subPath $index | quote }}
{{- end}}
  {{- $netList = (printf "%s,default/%s" $netList ((index $subPath $index))) }}
{{- end }}
copyNetPlane: {{ $copy | quote }}
  {{- $netList = (printf "%s,default/%s" $netList $copy) }}
{{- else if $backup }}
{{- $subPath:= (splitList ";" $backup) }}
{{- $l_num:= (len $subPath) }}
{{- range $index := untilStep 0 $l_num 1}}
{{- if eq $index 0}}
backupNetPlane: {{ index $subPath $index | quote }}
{{- else}}
backupNetPlane_{{ $index }}: {{ index $subPath $index | quote }}
{{- end}}
  {{- $netList = (printf "%s,default/%s" $netList ((index $subPath $index))) }}
{{- end }}
{{- else if $copy }}
  {{- $netList = (printf "%s,default/%s" $netList $copy) }}
copyNetPlane: {{ $copy | quote }}
{{- end}}
{{- if $storage }}
storageNetPlane: {{ $storage | quote }}
{{- $netList = (printf "%s,default/%s" $netList $storage) }}
{{- end }}
k8s.v1.cni.cncf.io/networks: "{{ $netList }}"
{{- end }}

{{/*
Create the networks annotation for a deployment or statefulset
*/}}
{{- define "sftp.networksAnnotation" -}}
{{- $netPlane := "" }}
{{- $netList := "default/data" }}
{{- if .Values.global.sftpNetPlane }}
{{- $netPlane = .Values.global.sftpNetPlane | toString }}
{{- end }}
{{- if $netPlane }}
sftpNetPlane: {{ $netPlane | quote }}
{{- $netList = (printf "%s,default/%s" $netList $netPlane) }}
{{- end }}
k8s.v1.cni.cncf.io/networks: "{{ $netList }}"
{{- end }}

{{/*
Create the container resource limits of frontend: sftp
*/}}
{{- define "limits.sftpNetPlane" -}}
{{- $netplaneId := "" }}
{{- if .Values.global.sftpNetPlane }}
{{- $netplaneId = .Values.global.sftpNetPlane | toString }}
{{- end }}
{{- if $netplaneId }}
dorado/dev_vf_{{ $netplaneId }}: 1
{{- end }}
{{- end }}

{{/*
Create the container resource limits of frontend: backup
*/}}
{{- define "limits.backupNetPlane" -}}
{{- $netplaneId := "" }}
{{- if .Values.global.backupNetPlane }}
{{- $netplaneId = .Values.global.backupNetPlane | toString }}
{{- end }}
{{- if $netplaneId }}
{{- $subPath:= (splitList ";" $netplaneId) }}
{{- $l_num:= (len $subPath) }}
{{- range $index := untilStep 0 $l_num 1}}
dorado/dev_vf_{{ ((index $subPath $index)) }}: 1
{{- end}}
{{- end }}
{{- end }}

{{/*
Create the container resource limits of frontend: archive
*/}}
{{- define "limits.archiveNetPlane" -}}
{{- $netplaneId := "" }}
{{- if .Values.global.archiveNetPlane }}
{{- $netplaneId = .Values.global.archiveNetPlane | toString }}
{{- end }}
{{- if $netplaneId }}
dorado/dev_vf_{{ $netplaneId }}: 1
{{- end }}
{{- end }}

{{/*
Create the container resource limits of frontend: copy
*/}}
{{- define "limits.copyNetPlane" -}}
{{- $netplaneId := "" }}
{{- if .Values.global.copyNetPlane }}
{{- $netplaneId = .Values.global.copyNetPlane | toString }}
{{- end }}
{{- if $netplaneId }}
dorado/dev_vf_{{ $netplaneId }}: 1
{{- end }}
{{- end }}

{{/*
Create the container resource limits of frontend: copy
*/}}
{{- define "limits.storageNetPlane" -}}
{{- $netplaneId := "" }}
{{- if .Values.global.storageNetPlane }}
{{- $netplaneId = .Values.global.storageNetPlane | toString }}
{{- end }}
{{- if $netplaneId }}
dorado/dev_vf_{{ $netplaneId }}: 1
{{- end }}
{{- end }}
