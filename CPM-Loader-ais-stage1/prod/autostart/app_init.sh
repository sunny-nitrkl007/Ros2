#!/bin/sh

#---------------------------------------------------------
# app_init.sh: this script is called by OS on the boot.
#---------------------------------------------------------
logger -p user.info "CPM: Begin app_init.sh"

# changes made to /etc/profile by other verities are not seen in following verities,
# so we need to manually do that
. /etc/profile 
# common source for CAT_DIR and $HOME on the ECM
. vehDataDir

SERVICE_DASH_WAIT_IN_SECONDS=5

IsServiceDashboardVerityPresent=false
IsCoreInfrastructureVerityPresent=false
IsMachineHealthVerityPresent=false

if ls ${vehDataHome}/ServiceDashboard*.verity 1> /dev/null 2>&1; then
   IsServiceDashboardVerityPresent=true
fi

if ls ${vehDataHome}/CoreInfrastructure*.verity 1> /dev/null 2>&1; then
   IsCoreInfrastructureVerityPresent=true
fi

if ls ${vehDataHome}/MachineHealth*.verity 1> /dev/null 2>&1; then
   IsMachineHealthVerityPresent=true
fi

#sleep till CPM verity is mounted.
while true
do
   if [ -d ${vehDataDir} ]
   then
      logger -s -p user.info "Found CPM verity."
      break;
   fi
   sleep 1;
done

#wait upto 5 seconds. for ServiceDashboard to be mounted
if [ "$IsServiceDashboardVerityPresent" = true ] ; then
  COUNT1=0
  until mountpoint -q ${vehDataHome}/ServiceDashboard
  do
    if [ "$COUNT1" -ge "$SERVICE_DASH_WAIT_IN_SECONDS" ]
    then
        logger -s -p user.info "timeout. waiting for service dashboard mount."
        break
    fi
    sleep 1
    let "COUNT1++"
  done
fi

# Fixing exec bit for /opt/ if this is DEV tokened ECM
if [ -f /opt/appdata/DeveloperToken/ct_token ]; then
    # remount /opt partition with exec bit set on
    mount -o remount,exec /opt
fi

# Fixing keyswitch
if [ "$IsMachineHealthVerityPresent" = false ] ; then
if [[ -f "/sys/class/rtc/rtc0/wakealarm_st" ]]; then
    if cat "/sys/class/rtc/rtc0/wakealarm_st" | grep -xqFe "1"; then
        echo 0 > /sys/class/rtc/rtc0/wakealarm_st
    fi
fi
fi

# 
# The following fix is needed because CAN performancy of the IMU 
# task is not acceptable unless this is set to low latency.
# 
# Set serial port to low_latency if ttyS1 is a character device
if [ -c "/dev/ttyS1" ]; then
    setserial /dev/ttyS1 low_latency
fi

# 
# The following fix is needed because switch to grounds do
# not work with certain versions of the A6N2 OS (ie 2019_1.0).
# This means location codes do not work unless this is set to 
# low latency.
# 
# Set serial port to low_latency if ttyS2 is a character device
if [ -c "/dev/ttyS2" ]; then
    setserial /dev/ttyS2 low_latency
fi

# Make sure all of our binaries, libraries and scripts are executable
if [ -w "${vehDataDir}" ]; then
   chmod -R 777 "${vehDataDir}"
fi

# Remove old file no longer being used.
AUTOSTART_DEBUG_FILE="/opt/appdata/CPM/debug/autostart.txt"
if [ -f $AUTOSTART_DEBUG_FILE ]; then
    rm -f $AUTOSTART_DEBUG_FILE
fi

# Remove old file no longer being used.
SYSAIS_LOG_DEBUG_FILE="/opt/appdata/CPM/debug/syslog.txt"
if [ -f $SYSAIS_LOG_DEBUG_FILE ]; then
    rm -f $SYSAIS_LOG_DEBUG_FILE
fi

# Set env variables for current script and child scripts
export CAT_DIR=${vehDataDir}
export CAT_CONFIG_DIR=${CAT_DIR}/config
if [[ "$LD_LIBRARY_PATH" == "" ]]; then
  export LD_LIBRARY_PATH=${CAT_DIR}/lib:${vehDataHome}/ossSoftware/usr/lib
else
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${CAT_DIR}/lib:${vehDataHome}/ossSoftware/usr/lib
fi
export HOME=${vehDataHome}

# Set env variables for all ssh sessions
echo "" >> /etc/profile
echo "# Set AIS env variables for all ssh sessions" >> /etc/profile
echo "export CAT_DIR=${vehDataDir}" >> /etc/profile
echo "export CAT_CONFIG_DIR=${CAT_DIR}/config" >> /etc/profile
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH" >> /etc/profile
echo "export HOME=${vehDataHome}" >> /etc/profile

# We have to have /bin/bash, since it is hardcoded in out bash scripts. copy only when not present in /bin/folder. 
if [ ! -f /bin/bash ]; then
if [ -f ${HOME}/ossSoftware/usr/bin/bash ]; then
  cp ${HOME}/ossSoftware/usr/bin/bash /bin/
else
  echo "*** ${HOME}/ossSoftware/usr/bin/bash is missing, exiting."
  exit 1
fi
fi

# We have to have /bin/rsync. copy only when not present in /bin/ folder.
if [ ! -f /bin/rsync ]; then
if [ -f ${HOME}/ossSoftware/usr/bin/rsync ]; then
  cp ${HOME}/ossSoftware/usr/bin/rsync /bin/
fi
fi

# We have to have /lib/libpopt.so.0. copy only when not present in /lib/ folder.
if [ ! -f /lib/libpopt.so.0 ]; then
if [ -f ${HOME}/ossSoftware/usr/lib/libpopt.so.0 ]; then
  cp ${HOME}/ossSoftware/usr/lib/libpopt.so.0 /lib/
fi
fi


# robot doesnt exist  
if ! [ -f ${HOME}/etc/robot ]; then
  if ! [ -d ${HOME}/etc ]; then
    mkdir -p ${HOME}/etc/
  fi

  echo "NOT00000" > /opt/etc/robot.tmp         #NOT00000 is default config
  mv -f /opt/etc/robot.tmp /opt/etc/robot

# robot exist and empty    
elif ! [ -s ${HOME}/etc/robot ]; then
  echo "NOT00000" > /opt/etc/robot.tmp         #NOT00000 is default config
  mv -f /opt/etc/robot.tmp /opt/etc/robot

# robot exist and not empty
else
  echo "robot exist and not empty"
fi



################################################################################
# The following is disabled because it takes too long to replay the device
# events from the start just in case there is a usb drive mounted, which is
# unlikely anyway. If this is ever needed again, consider a couple of options:
# 1 - Only trigger udev add if ls /dev/sd[a-z][0-9] finds something
# 2 - When triggering, filter down to "block" subsystem --subsystem-match=block
################################################################################
#
## Create a new udev rule to mount the log disk whenever a new usb hard drive is connected
####
#cat <<EOF >/etc/udev/rules.d/91-mount_log_disk.rules
## If kernel detects a usb hard drive device 
#KERNEL!="sd[a-z][0-9]", GOTO="mount_log_disk_end"
#
## Import FS infos for device information
#IMPORT{program}="/sbin/blkid -o udev -p %N"
#
## Mount the connected device to /opt/log when the kernel detects a hard drive has been connected
#ACTION=="add", ENV{ID_FS_LABEL}=="logs", RUN+="/usr/bin/logger 'Mounting Log Disk.'"
#ACTION=="add", ENV{ID_FS_LABEL}=="logs", RUN+="/bin/mount -o relatime /dev/%k /opt/log"
#
## Clean up after removal
#ACTION=="remove", ENV{ID_FS_LABEL}=="logs", RUN+="/bin/umount -l /opt/log"
#
## Exit
#LABEL="mount_log_disk_end"
#EOF
####
#
## Load any rules created since startup
#udevadm control --reload-rules
## Trigger the "add" ACTION to mount any hard drive connected at startup
#udevadm trigger --action=add 

#sleep till network is up
if [ "$IsCoreInfrastructureVerityPresent" = true ] ; then
	while true
	do
	   if [ -f "/tmp/net_up" ]
	   then
	      logger -s -p user.info "Found net up. Proceeding to start..."
	      break;
	   fi
	   sleep 1;
	done
fi

# Modifying HostName
if [ -f "/opt/appdata/DefaultConfig/config/hostname" ]
then
    new_host_name=$( cat /opt/appdata/DefaultConfig/config/hostname )
    host_name=$(cat /etc/hostname)
    hostname $new_host_name
    sed -i "s/$host_name/$new_host_name/g" /etc/hosts
    sed -i "s/$host_name/$new_host_name/g" /etc/hostname
fi

# Launch the RPA Client application only when MachineHealth is not present
if [ "$IsMachineHealthVerityPresent" = false ] ; then
if [ -f ${CAT_DIR}/bin/rpa_client ]; then
 screen -h 1000 -d -m -S rpa_client ${CAT_DIR}/bin/rpa_client 
fi
fi

# Make sure our appdata exists, if not, create it
if [ ! -d /opt/appdata/CPM ]
then
	mkdir -p /opt/appdata/CPM
fi

# Launch the portForwardingMgr to enable port forwarding
# config for portForwarding can be found in $CAT_CONFIG_DIR/port_forwarding_cfg.xml
if [ -f "${CAT_DIR}/bin/portForwardingMgr" ]; then
 screen -h 1000 -d -m -S portForwardingMgr "${CAT_DIR}/bin/portForwardingMgr"
fi

# Launch system restart task
if [ -f ${CAT_DIR}/bin/systemRestart ]; then
 screen -h 1000 -d -m -S systemRestart ${CAT_DIR}/bin/systemRestart 
fi

# Start AIS app
${CAT_DIR}/scripts/autostart

logger -p user.info "CPM: After autostart"

# The following can be used to monitor the bootup process.
#top -b -n 20 -d 2 > /opt/tmp/outputFiles/top_dump.txt

# Sleeping to let the important stuff finish in peace before we
# do the less important stuff.
sleep 30

# Attempt to determine the IP address and start another instance of FTP server
# BINDIP=$(ifconfig eth0.20 | grep "inet addr" | cut -d : -f 2 | cut -d " " -f 1)
# setup two FTP servers for hlogs & nvm
BINDIP=165.26.79.19
if [ -n "$BINDIP" ]; then
  FTPCFG="${CAT_CONFIG_DIR}/vsftpd_logger.conf"
  if [ -f "${FTPCFG}" ]; then
    vsftpd "${FTPCFG}" -olisten_address="${BINDIP}"
    echo "Found IP: ${BINDIP}"
  else
    echo "Unable to locate ${FTPCFG}"
  fi
else
  echo "Could not obtain IP to bind for eth0"
fi

if [ -n "$BINDIP" ]; then
  FTPCFG="${CAT_CONFIG_DIR}/vsftpd_nvm.conf"
  if [ -f "${FTPCFG}" ]; then
    vsftpd "${FTPCFG}" -olisten_address="${BINDIP}"
    echo "Found IP: ${BINDIP}"
  else
    echo "Unable to locate ${FTPCFG}"
  fi
else
  echo "Could not obtain IP to bind for eth0"
fi

if [ -n "$BINDIP" ]; then
  FTPCFG="${CAT_CONFIG_DIR}/vsftpd_outputFiles.conf"
  if [ -f "${FTPCFG}" ]; then
    vsftpd "${FTPCFG}" -olisten_address="${BINDIP}"
    echo "Found IP: ${BINDIP}"
  else
    echo "Unable to locate ${FTPCFG}"
  fi
else
  echo "Could not obtain IP to bind for eth0"
fi

logger -p user.info "CPM: End app_init.sh"

