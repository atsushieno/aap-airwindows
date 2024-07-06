#!/bin/sh

echo '<plugins xmlns="urn:org.androidaudioplugin.core" xmlns:pp="urn:org.androidaudioplugin.port">'

for f in external/airwin2rack/src/autogen_airwin/*.h ; do
	filename=$(basename -- $f)
	NAME="${filename%.*}"
	echo "  <plugin name='${NAME}' category='Effect' developer='AAP Porting Project' unique-id='urn:airwindows:${NAME}:v1' library='libaap-airwindows.so' entrypoint='GetAndroidAudioPluginFactory' gui:ui-view-factory='org.androidaudioplugin.ui.compose.ComposeAudioPluginViewFactory' xmlns:gui='urn://androidaudioplugin.org/extensions/gui'>"
	echo '    <extensions>'
	echo '      <extension uri="urn://androidaudioplugin.org/extensions/plugin-info/v3" />'
      	echo '      <extension uri="urn://androidaudioplugin.org/extensions/state/v3" />'
	echo '      <extension uri="urn://androidaudioplugin.org/extensions/parameters/v3" />'
	echo '      <extension uri="urn://androidaudioplugin.org/extensions/midi/v3" />'
	echo '      <extension uri="urn://androidaudioplugin.org/extensions/gui/v3" />'
	echo '    </extensions>'
	echo '  </plugin>' ;
done
echo '</plugins>'
