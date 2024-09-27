FROM open-ebackup-1.0:base

WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_data

ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/lib64:/usr/lib:/usr/local/lib:\
/opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_data/lib:/opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_data/lib/3rd

RUN useradd dme_openstorageapi_data -M -N -s /sbin/nologin  -u 19864 -g 99
RUN chown -R 19864:99 /opt/OceanStor && chmod 750 /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_data
RUN rm -f /usr/lib64/libkmcv3.so

COPY --chown=19864:99 /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_data /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_data

USER 19864
CMD [ "/opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_data/bin/OsadMgr" ]