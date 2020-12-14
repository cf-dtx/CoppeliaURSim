# Coppelia_UR_plugin

This is a plugin that allows you to visualize Universal Robots programs in the CoppeliaSim simulator. You can find a setup guide and an example below.

# Get the URSim 

*This setup was tested with a Ubuntu 18.04 LTS operating system.*

First step, download the URSim simulator from this link - [ursim-5.8.2](https://www.universal-robots.com/download/?option=71479#section41511).
Next, unzip the contents of the zip to a folder, let's call it, the `root` folder.
For the next steps of the manual, make sure you are in the `root` folder.
```sh
$ cd (...)/root
```

## Install Java 8

By default, the Java debian package for Ubuntu 18.04 installs Java version 11. If you have already installed Java, you should get the following output:

```sh
$ java -version
openjdk version "11.0.7" 2020-04-14
OpenJDK Runtime Environment (build 11.0.7+10-post-Ubuntu-2ubuntu218.04)
OpenJDK 64-Bit Server VM (build 11.0.7+10-post-Ubuntu-2ubuntu218.04, mixed mode, sharing)

```
Yet, and for the URSim to launch you will need to shift to Java 8. First install the version 8:

If you have never installed java in your machine, when you write theese commands, you should get the following output:
```sh
$ sudo apt update
$ sudo apt install openjdk-8-jdk openjdk-8-jre
$ java -version
openjdk version "1.8.0_252"
OpenJDK Runtime Environment (build 1.8.0_252-8u252-b09-1ubuntu1-b09)
OpenJDK 64-Bit Server VM (build 25.252-b09, mixed mode)
```
If you have any other java version installed you need to specifically tell your machine which version to use by doing so:

```sh
$ sudo update-alternatives --config java
There are 2 choices for the alternative java (providing /usr/bin/java).

  Selection    Path                                            Priority   Status
------------------------------------------------------------
* 0            /usr/lib/jvm/java-11-openjdk-amd64/bin/java      1111      auto mode
  1            /usr/lib/jvm/java-11-openjdk-amd64/bin/java      1111      manual mode
  2            /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java   1081      manual mode

Press <enter> to keep the current choice[*], or type selection number: 2
update-alternatives: using /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java to provide /usr/bin/java (java) in manual mode

```

## Install XML RPC C++ 32-bit library

You install the XML RPC C++ 32-bit library following the command:
```sh
sudo apt install libxmlrpc-c++8v5:i386
```

## Install script

:warning: Before running the installation script, keep in mind that if you already have ROS installed, it will remove some of the installed libraries. :warning:

We suggest installing the URSim prior to ROS. If you cannot do that, we suggest adjusting the `install.sh` script to the following:

```bash
#!/bin/bash
userServiceDirectory() {
	echo "$URSIM_ROOT/service"
}

userDaemonManagerDirectory() {
	echo "/etc/runit/runsvdir-ursim-$VERSION"
}

installDaemonManager() {
	local userServiceDirectory=`userServiceDirectory`
	local userDaemonManagerDirectory=`userDaemonManagerDirectory`
	local userDaemonManagerRunScript="$userDaemonManagerDirectory/run"

	echo "Installing daemon manager package"
	# if it fails comment out, and check answer https://askubuntu.com/a/665742
	sudo apt install runit

	echo "Creating user daemon service directory"
	sudo mkdir -p $userDaemonManagerDirectory
	echo '#!/bin/sh' | sudo tee $userDaemonManagerRunScript >/dev/null
	echo 'exec 2>&1' | sudo tee -a $userDaemonManagerRunScript >/dev/null
	echo "exec chpst -u`whoami` runsvdir $userServiceDirectory" | sudo tee -a $userDaemonManagerRunScript >/dev/null
	sudo chmod +x $userDaemonManagerRunScript

	echo "Starting user daemon service"
	sudo ln -sf $userDaemonManagerDirectory /etc/service/
	mkdir -p $userServiceDirectory
}

jdk_version() {
  local result
  local java_cmd
  if [[ -n $(type -p java) ]]
  then
    java_cmd=java
  elif [[ (-n "$JAVA_HOME") && (-x "$JAVA_HOME/bin/java") ]]
  then
    java_cmd="$JAVA_HOME/bin/java"
  fi
  local IFS=$'\n'
  # remove \r for Cygwin
  local lines=$("$java_cmd" -Xms32M -Xmx32M -version 2>&1 | tr '\r' '\n')
  if [[ -z $java_cmd ]]
  then
    result=no_java
  else
    for line in $lines; do
      if [[ (-z $result) && ($line = *"version \""*) ]]
      then
        local ver=$(echo $line | sed -e 's/.*version "\(.*\)"\(.*\)/\1/; 1q')
        # on macOS, sed doesn't support '?'
        if [[ $ver = "1."* ]]
        then
          result=$(echo $ver | sed -e 's/1\.\([0-9]*\)\(.*\)/\1/; 1q')
        else
          result=$(echo $ver | sed -e 's/\([0-9]*\)\(.*\)/\1/; 1q')
        fi
      fi
    done
  fi
  echo "$result"
}

needToInstallJava() {
    echo "Checking java version"
    if command -v java; then
	# source https://stackoverflow.com/questions/7334754/correct-way-to-check-java-version-from-bash-script
        version=$(java -version 2>&1 | awk -F '"' '/version/ {print $2}')
        echo version "$version"
        if [[ "$version" > "1.6" ]] ; then
	    echo "java version accepted"
            return 0
	fi
    fi
    return 0
}

# if we are not running inside a terminal, make sure that we do
tty -s
if [[ $? -ne 0 ]]
then
	xterm -e "$0"
	exit 0
fi

needToInstallJava
if [[ $? -ne 0 ]]; then
	# install default jre for distribution, make sure that it's at least 1.6
	sudo apt-get install default-jre
	if [[ $? -ne 0 ]]; then
		echo "Failed installing java, exiting"
		exit 2
	fi
	needToInstallJava
	if [[ $? -ne 0 ]]; then
		echo "Installed java version is too old, exiting"
		exit 3
	fi
fi

set -e

## COMMENTED - INSTALL MANUALLY
#commonDependencies='libcurl3 openjdk-8-jre libjava3d-* ttf-dejavu* fonts-ipafont fonts-baekmuk fonts-nanum fonts-arphic-uming fonts-arphic-ukai'
#if [[ $(getconf LONG_BIT) == "32" ]]
#then
#	Dependencies_32='libxmlrpc-c++8 libxmlrpc-core-c3'
#	pkexec bash -c "apt-get install $commonDependencies $Dependencies_32"
#else
#        #Note: since URController is essentially a 32-bit program
#        #we have to add some 32 bit libraries, some of them picked up from the linux distribution
#        #some of them are have been recompiled and are inside our ursim-dependencies directory in deb format
#	packages=`ls $PWD/ursim-dependencies/*amd64.deb`
#	pkexec bash -c "apt-get install lib32gcc1 lib32stdc++6 libc6-i386 $commonDependencies && (echo '$packages' | xargs dpkg -i --force-overwrite)"
#fi

source version.sh
URSIM_ROOT=$(dirname $(readlink -f $0))

echo "Install Daemon Manager"
installDaemonManager

for TYPE in UR3 UR5 UR10 UR16
do
	FILE=$HOME/Desktop/ursim-$VERSION.$TYPE.desktop
	echo "[Desktop Entry]" > $FILE
	echo "Version=$VERSION" >> $FILE
	echo "Type=Application" >> $FILE
	echo "Terminal=false" >> $FILE
	echo "Name=ursim-$VERSION $TYPE" >> $FILE
	echo "Exec=${URSIM_ROOT}/start-ursim.sh $TYPE" >> $FILE
	echo "Icon=${URSIM_ROOT}/ursim-icon.png" >> $FILE
	chmod +x $FILE
done

pushd $URSIM_ROOT/lib &>/dev/null
chmod +x URControl

popd &>/dev/null

```

You will notice that there is a block of code commented comparing to the original one. This will prevent the script from installing the commented libraries, but it will also prevent it from deleting other libraries. We suggest installing the above mentioned dependencies prior to running the script.

If you run a 64-bit system:
```sh
$ sudo apt install libcurl4 libjava3d-* ttf-dejavu* fonts-ipafont fonts-baekmuk fonts-nanum fonts-arphic-uming fonts-arphic-ukai libxmlrpc-c++8 libxmlrpc-core-c3
```
else,
```sh
$ sudo apt install libcurl4 libjava3d-* ttf-dejavu* fonts-ipafont fonts-baekmuk fonts-nanum fonts-arphic-uming fonts-arphic-ukai lib32gcc1 lib32stdc++6 libc6-i386
```
You need to install the dependencies inside the `root/ursim-dependencies` folder as well. 

Now, you can run the installation script:
```sh
$ ./install.sh
```
If you do not have the permission to run this or any of the other script files, you should run the following command:
```sh
$ sudo chmod +x file.sh
```

# Launch the Simulator

If you followed the steps above, hopefully, the simulator can be launched by the desktop file, it creates in your `~/Desktop` folder, or by typing in a terminal:
```sh
$ ./start-ursim.sh
```

## Create the `net-statistics` file

For reasons we are unaware of, the URSim program requires but does not install an additional script, responsible for checking the current network status, and needed to make the simulator reachable through the network.

You should open your favorite editor with super-user privileges and create a new file where you paste the following:

```perl
#!/usr/bin/perl
 
my $interface = "wlp4s0";
#my $interface = "eth1";
my $isDhcp = 1;
my $isStatic = 0;
 
open(INTF,"</etc/network/interfaces") || die "Cannot open interfaces";
while($line = <INTF>) {
    chomp $line;
    if ($line =~/eth0/) {
        $interface = "eth0";
    }
    if ($line =~/wlp4s0/) {
        $interface = "wlp4s0";
    }
    if ($line =~ /$interface.*dhcp/) {
        $isDhcp = 1;
    } elsif ($line =~ /$interface.*static/) {
        $isStatic = 1;
    }
}
close INTF;
 
my $interface_operstate = `cat /sys/class/net/$interface/operstate`;
my @ifconfig = split(/\n/,`/sbin/ifconfig $interface 2>&1`);
 
my $addr = "";
my $mask = "";
my $ifconfig_str = join(" ", @ifconfig);
my $isNetDown = 1;
 
 
if ($interface_operstate =~ /up/) {
    $isNetDown = 0;
}
 
if ($isDhcp) {
    print "Mode:dhcp\n";
} elsif ($isStatic) {
    print "Mode:static\n";
} else {
    print "Mode:disabled\n";
}
 
if ($isNetDown) {
    print "Net down\n";
} else {
    print "Net up\n";
}
 
 
foreach $line (@ifconfig) {
    chomp $line;
    if($line =~ /inet /) {
    $addr = $line;
    $addr =~ s/.*inet ([^ ]*).*/$1/;
    }
    if($line =~ /netmask /) {
    $mask = $line;
    $mask =~ s/.*netmask ([^ ]*).*/$1/;
    }
}
 
my @route = split(/\n/,`/sbin/route -n`);
my $gateway = "";
foreach $line (@route) {
    chomp $line;
    if($line =~ /^0.0.0.0 /) {
        $gateway = $line;
        $gateway =~ s/^0.0.0.0[ ]*([^ ]*).*/$1/;
    }
}
 
open RES, "</etc/resolv.conf";
my @nameservers;
while($line = <RES>) {
    chomp $line;
    if($line =~ /nameserver/) {
        my $name=$line;
        $name =~ s/nameserver //;
        push @nameservers, $name;
    }
}
close RES;
 
my @hostname = split(/\n/, `hostname`);
 
print "Address:$addr\n";
print "Mask:$mask\n";
print "Gateway:$gateway\n";
print "nameserver1:" . $nameservers[0] . "\n";
print "nameserver2:" . $nameservers[1] . "\n";
print "Hostname:$hostname[0]\n";
```

You should save this file directly in the `/sbin` directory, which is where the URSim will look for it. 

What it does is to sniff your current network connection and fetch the details in a formatted output to the URSim application. You should run the following commands:

```sh
$ cd /sbin
$ sudo chown <user>:<user> net-statistics
$ sudo chmod +x net-statistics
```

where `<user>` is your pc user name. 
When you call the scrip, you should get information about your connection in the following format:

```sh
$ /sbin/net-statistics 
Mode:dhcp
Net up
Address:192.168.1.99
Mask:255.255.255.0
Gateway:192.168.1.254
nameserver1:127.0.0.53
nameserver2:
Hostname:<pc_name>
```

The connection values vary depending on the network you are connected to. 

From the `net-statistics` file, there are two important lines:

```perl
(...)

my $interface = "wlp4s0";
my $isDhcp = 1;

```

When you type `iwconfig` you will get the name of the interface of the different network connections your host has available. The `lo` interface refers to the local network, `eth0` refers to the wired network, and some variation of `wlan` should refer to the wireless network interface.

In our case, the wireless network name is `wlp4s0` and thus the name of that variable. 

**TODO:** update the file to automatically update this variable with the name of the network interface.

With this file in place, now you should be able to access the URSim through the network, for example to connect using the RTDE interface. 

# RTDE

To connect to the URSim using the RTDE interface, follow the guide at:

https://sdurobotics.gitlab.io/ur_rtde/

# CoppeliaSim Plugin Setup

Clone this repository to a directory of choice, let it be `<dir>`
Create a `build` folder and compile the source code.

```sh
$ cd <dir>
$ mkdir build; cd build
$ cmake ..
$ make -j8
``` 

Inside the `<dir>/build` folder you will find a `libsimExtRTDE.so` file.

You need to copy this file and place it in the CoppeliaSim root directory (where you have the models, scenes and other folders, as well as other plugin libs with the name `libsimExt_.so`).

After you do that, and when you launch the CoppeliaSim, you can check the terminal,

```sh
CoppeliaSim Edu V4.0.0. (rev. 4)
Using the default Lua library.
Loaded the video compression library.
Add-on script 'simAddOnScript-addOnScriptDemo.lua' was loaded.
Add-on script 'simAddOnScript-b0RemoteApiServer.lua' was loaded.
Add-on script 'simAddOnScript_PyRep.lua' was loaded.
If CoppeliaSim crashes now, try to install libgl1-mesa-dev on your system:
>sudo apt install libgl1-mesa-dev
OpenGL: NVIDIA Corporation, Renderer: GeForce RTX 2060 SUPER/PCIe/SSE2, Version: 4.6.0 NVIDIA 450.51.06
...did not crash.
Simulator launched.
Plugin 'Geometric': loading...
Plugin 'Geometric': load succeeded.
Plugin 'Assimp': loading...
Plugin 'Assimp': warning: replaced variable 'simAssimp'
Plugin 'Assimp': load succeeded.
Plugin 'BlueZero': loading...
Plugin 'BlueZero': warning: replaced variable 'simB0'
(...)
Plugin 'RTDE': loading...
Plugin 'RTDE': load succeeded.
(...)
```

For these lines, 

```sh
Plugin 'RTDE': loading...
Plugin 'RTDE': load succeeded.
```

# Example Scene

To exemplify how this plugin works, let's launch an both URSim and CoppeliaSim applications.

## Start URSim

Launch URSim app, open a new terminal and type this,

```sh
$ ./start-ursim.sh
```

Then you can follow these steps to check if the URSim is able to connect with external applications,

![](https://imgur.com/9w8boGl.gif)

## Start CoppeliaSim

We included an example CoppeliaSim scene where you can test this plugin.
For easier access, copy the scene at `<dir>/scenes/CoppeliaSimRTDE_example.ttt` to the CoppeliaSim/scenes folder.

Then you can load and start the simulation, you should get the following output.

![](https://i.imgur.com/Wj66zhJ.gif)

If all goes well, now you can start creating and running programs in URSim, and you will see its output in the CoppeliaSim scene,

![](https://imgur.com/yPguXYm.gif)

That's all folks! :+1: