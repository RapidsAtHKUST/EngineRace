#!/usr/bin/env bash

teamCode=$1

logfile="log_${teamCode}.tar"

baseUrl="http://polardbrace.oss-cn-shanghai.aliyuncs.com/${logfile}"

dateFolderName=`date +%F`

baseDIR="logs-all/${dateFolderName}/${teamCode}"

mkdir -p ${baseDIR}

folderName=`date +%F_%H-%M`
latestFolder="${baseDIR}/${folderName}"

echo "download log into folder ${baseDIR}..."

cd ${baseDIR}

fileURL="$baseUrl"
echo ${fileURL}

#backup the old file timestamp

touch -r ${logfile} timestamp.old

wget ${fileURL}

mkdir -p ${folderName}
tar xvf ${logfile} -C ${folderName}
