# OXYGEN MQTT-Plugin

The latest documentation can be found on GitHub Pages https://dewetron.github.io/OXYGEN-SDK-MQTT/.

## Debug and Build the Plugin

To get started, Visual Studio Code with the following extensions can be used:

- The official C/C++ extension
- CMake Tools
- C++ TestMate to run Unit-Tests directly inside VS-Code

If you have OXYGEN installed on the default path and you start debugging, the compiled plugin gets automatically copied to the correct folder and OXYGEN is started. The Debugger will then attach to OXYGEN, allowing you to debug the plugin directly in VS-Code. To find out how this is done or how to change the default build-settings using the CMake extension, have a look at launch.json, settings.json and tasks.json.

### Ubuntu 22.04
In order to build the plugin on Ubuntu 22.04, you need to install the following development libraries:

```
sudo apt install -yq cmake g++ libfmt-dev libpaho-mqtt-dev libpaho-mqttpp-dev qtbase5-dev uuid-dev
```

You can build the plugin using any IDE with CMake support.
Or, to manually build the plugin from the command line, open a terminal in the folder where the MQTT plugin source is cloned into.
Then, execute the following commands to build the plugin:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target mqtt
```

To use the plugin in Oxygen, copy the `libmqtt.plugin` file from the `build/mqtt-plugin/Release/plugin` folder to `/usr/Oxygen/plugins`.
