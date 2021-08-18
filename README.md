# Maya To Gameplay3D Viewer Application

Instructions:

- Start the viewer application found in the "Done application"-folder.
- Load the maya plugin in Maya 2019. The plugin can also be found in the "Done application"-folder.
You are done!

--------

For building the plugin yourself:

- Open the visual studio project located inside the "MayaViewer_UD1447"-folder
- Open Maya 2019 and write the following into the MEL-script editor: commandPort -n ":1234"
- Build the ComLibForMaya-project in visual studio
- Build the plugin in visual studio (after configuring output directories)
- Build the viewer application in visual studio.
You are done!
