"c:\Program Files (x86)\Poedit\GettextTools\bin\xgettext.exe" -f .\src.list -o .\bitswash.pot -LC -k_

"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\de_DE.po .\bitswash.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\nb_NO.po .\bitswash.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\ru_RU.po .\bitswash.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\zh_CN.po .\bitswash.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\zh_TW.po .\bitswash.pot
del .\*.po~