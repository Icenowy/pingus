cat ../../po/zh_CN.po | ./buildset.py > out
./substractchars.py characters.txt out > characters-cs.txt
cat ../../po/zh_TW.po | ./buildset.py > out
./substractchars.py characters.txt out > characters-ct.txt
cat ../../po/ja.po | ./buildset.py > out
./substractchars.py characters.txt out > characters-ja.txt
cat characters-cs.txt characters-ct.txt characters-ja.txt | sort | LC_ALL=C uniq > characters-cjk.txt
rm characters-cs.txt characters-ct.txt characters-ja.txt

./fontgen  generate ukai.ttc 16 0 512 5000 "$(cat characters-cjk.txt)"
head -n 545 < chalk-16px.font > /tmp/1
cat /tmp/out.font | tail -n +4 | head -n -1 > /tmp/2
tail -n 2 < chalk-16px.font > /tmp/3
cat /tmp/1 /tmp/2 /tmp/3 > chalk-16px.font
cp /tmp/out.png chalk-cjk-16px.png
./fontgen  generate ukai.ttc 20 0 512 5000 "$(cat characters-cjk.txt)"
head -n 546 < chalk-20px.font > /tmp/1
cat /tmp/out.font | tail -n +4 | head -n -1 > /tmp/2
tail -n 2 < chalk-20px.font > /tmp/3
cat /tmp/1 /tmp/2 /tmp/3 > chalk-20px.font
cp /tmp/out.png chalk-cjk-20px.png
./fontgen  generate ukai.ttc 40 0 1024 5000 "$(cat characters-cjk.txt)"
head -n 546 < chalk-40px.font > /tmp/1
cat /tmp/out.font | tail -n +4 | head -n -1 > /tmp/2
tail -n 62 < chalk-40px.font > /tmp/3
cat /tmp/1 /tmp/2 /tmp/3 > chalk-40px.font
cp /tmp/out.png chalk-cjk-40px.png
./fontgen  generate ukai.ttc 20 1 512 5000 "$(cat characters-cjk.txt)"
head -n 547 < pingus-small-20px.font > /tmp/1
cat /tmp/out.font | tail -n +4 | head -n -1 > /tmp/2
tail -n 3 < pingus-small-20px.font > /tmp/3
cat /tmp/1 /tmp/2 /tmp/3 > pingus-small-20px.font
cp /tmp/out.png pingus-small-cjk-20px.png
