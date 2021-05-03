# Build environment setup for .NET 4.7.2 and WDK
This file contains instructions for setting up a build environment for bleedblack to be buiilt without using the desktop version of Microsoft Visual Studio.  
You might need to adjust the Windows-Core base image in the Dockerfile depending on what version of Windows you are currently running. Generally it should work 
on most up-to-date versions of Windows- 10 and Server 2019.

# Let's get started
First build a docker image that has all the necessary build tools installed:
```
docker build -t buildtools2019:latest -m 2GB .
```
Increase the 2GB memory limit to your liking. The default 2GB should be enough for almost all kinds of projects though.

## This is it
You should be all set now and ready to build drivers with this image

# GitLab-Runner integration
There is no direct integration for GitLab CI/CD pipelines but we can make it work with setting up a local docker registry that is available to the host that runs the runner.  

### Insert new entry into your hosts file
The content should be `127.0.0.1 local.registry`.

### Spin up a local registry 
First create a new directory at `C:\Registry`, then start a new registry inside a docker container:  
``docker run -d -p 5000:5000 --restart=always --name registry -v C:\registry:C:\registry stefanscherer/registry-windows:2.6.2``

### Tagging and pushing the build image to a local registry
```
docker tag buildtools2019:latest local.registry:5000/buildtools2019:latest
docker push local.registry:5000/buildtools2019:latest
```
That's it!

Now a GitLab runner that is local to the host of the registry will be able to pull the buildtools image from the local registry.
