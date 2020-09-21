out_path=/tmp/local_tts.audio
cur_tex="你没有输入文字"
cur_per=0 #0为普通女声，1为普通男生，3为情感合成-度逍遥，4为情感合成-度丫丫，默认为普通女声
cur_spd=6 #语速，取值0-9，默认为5中语速
cur_pit=5 #音调，取值0-9，默认为5中语调
cur_vol=5 #音量，取值0-9，默认为5中音量
cur_aue=3 #下载的文件格式, 3：mp3	4：pcm-16k  5：pcm-8k  6. wav
while getopts "St:o:" args
do
	case $args in
		S)      killall -9 ${0##*/}
				;;
		o)      out_path=$OPTARG
				;;
		t)		cur_tex=$OPTARG
				;;
		h)      echo "  $0 help:"
				echo "  -h              :show help" 
				return 0  
				;;
		?)      echo "unkonw argument"
	exit 1
	;;
	esac
done
token_url="http://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=4E1BG9lTnlSeIf1NQFlrSq6h&client_secret=544ca4657ba8002e3dea3ac2f5fdd241"
token_res0=`curl $token_url 2>/dev/null`
token_res1=${token_res0##*'access_token":"'}
token_res=${token_res1%%'",'*}
speechTxt_url="http://tsn.baidu.com/text2audio?ctp=1&lan=zh&cuid=1234567C&tok=$token_res&tex=$cur_tex&per=$cur_per&spd=$cur_spd&pit=$cur_pit&vol=$cur_vol&aue=$cur_aue"
curl -o $out_path $speechTxt_url 2>/dev/null
