#!/bin/sh

echo '<plugins xmlns="urn:org.androidaudioplugin.core" xmlns:pp="urn:org.androidaudioplugin.port">'

# AirwinRegistry::registerAirwindow() skips entries with an empty description.
sed -n '/, [a-z]*, "",/d; s/.*AirwinRegistry::registerAirwindow({"\([^"]*\)".*/\1/p' external/airwin2rack/src/ModuleAdd.h |
while IFS= read -r NAME ; do
	echo "  <plugin name='${NAME}' category='Effect' developer='AAP Porting Project' unique-id='urn:airwindows:${NAME}:v1' library='libaap-airwindows.so' entrypoint='GetAndroidAudioPluginFactory' gui:ui-view-factory='org.androidaudioplugin.ui.compose.ComposeAudioPluginViewFactory' xmlns:gui='urn://androidaudioplugin.org/extensions/gui'>"
	echo '    <extensions bom="0.12.1" />'
	echo '  </plugin>' ;
done
echo '</plugins>'
