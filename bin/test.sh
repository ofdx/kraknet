#!/bin/sh
export SERVER_NAME="anubis.krakissi.net"
export web_user_name="mike"
export web_root="/home/mike/src/ws/web"
export DOCUMENT_ROOT=$web_root
export mod_root="/home/mike/src/ws/mods"
export conf_dir="/home/mike/src/ws/conf"
export tmp_ws="/tmp/krakws"

bin_path="/home/mike/src/ws/bin"
export PATH="$PATH:$bin_path"

bash
