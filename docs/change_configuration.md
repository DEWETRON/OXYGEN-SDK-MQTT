# Changing Plugin Configurations 
Once the plugin has been added to OXYGEN, you cannot edit any MQTT-related properties directly from within OXYGEN. However, whenever you start OXYGEN, the current configuration is exported to the following path:

```
C:\Users\Public\Documents\Dewetron\Oxygen\Plugins\{config-file-name}.json.cache
```

When you reload the plugin, it will try to load the configuration from this cache file. If you make any changes here, these changes will be reflected within OXYGEN. You can even add or delete a topic. 

Never change the `__uuid` properties as these are needed to match topics to existing OXYGEN channels.