#
# HW5b_script.sh
# usage: test basic functions and additional implementations of nsh
# name: wing wing
#

echo "testing pwd, mkdir, cd ..."
echo "$ pwd";                                             pwd
#mkdir dir1                        #create a directory
echo "$ cd dir1";                                         cd dir1
echo "$ pwd";                                             pwd
echo "$ cd ../";                                          cd ../
echo "$ pwd";                                             pwd

echo
echo "testing meta character(s) ..."
echo "$ who";                                             who
echo "$ who|wc";                                          who|wc
echo "$ who | wc";                                        who | wc
echo "$ who | wc -l";                                     who | wc -l

echo
echo "testing redirection & pipe(s) ..."
echo "$ ls -l > output.txt";                              ls -l > output.txt
echo "$ cat output.txt";                                  cat output.txt
echo "$ cat output.txt | head -7 | tail -3";              cat output.txt | head -7 | tail -3
echo "$ cat output.txt|head -7|tail -3";                  cat output.txt|head -7|tail -3
echo "$ cat output.txt|head -7|tail -3|wc";               cat output.txt|head -7|tail -3|wc
echo "$ ls -l|head -5|tail -n +5|sort|fgrep 1|uniq -c";   ls -l|head -5|tail -n +5|sort|fgrep 1|uniq -c
echo "$ wc -l output.txt";                                wc -l output.txt
echo "$ wc -l < output.txt > newfile.txt";                wc -l < output.txt > newfile.txt
echo "$ cat newfile.txt";                                 cat newfile.txt
echo "$ ls -l newfile.txt >> newfile.txt";                ls -l newfile.txt >> newfile.txt
echo "$ cat newfile.txt";                                 cat newfile.txt

echo
echo "testing rm, cp, mv ..."
echo "$ cp newfile.txt fileToRemove.txt";                 cp newfile.txt fileToRemove.txt
echo "$ mv fileToRemove.txt dir1/";                       mv fileToRemove.txt dir1/
echo "$ ls dir1/";                                        ls dir1/
echo "$ rm dir1/fileToRemove.txt";                        rm dir1/fileToRemove.txt
echo "$ ls dir1/";                                        ls dir1/

echo
echo "testing comment(s) & quote(s) ..."
echo "$ echo foo #I am a foo haha";                       echo foo #I am a foo haha
echo "$ echo foo";                                        echo foo
echo "$ echo 'I am a foo haha'";                          echo 'I am a foo haha'
echo "$ echo '\ b \ '";                                   echo '\ b \ '
echo "$ echo \"I am a foo haha\"";                        echo "I am a foo haha"

echo
echo "testing escape character(s) ..."
echo "$ ab\c";                                            echo ab\c

echo
echo "testing background execution ..."
echo "$ echo hello world &";                              echo hello world &
echo "$ ls -S &";                                         ls -S &
echo "$ echo \|";                                         echo \|
