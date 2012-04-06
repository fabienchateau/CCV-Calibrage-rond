#!/bin/sh

dirInstallUnicap=~/installUnicap
userChoice="n"
userCheck=1
versionUnicap="unicap-0.9.5"
nomPatch="unicap-gspca.patch"
downloadSource=1

#Test des parametres
if [ "$1" != "" ]; then
	if [ "$1" = "-f" ]; then
		userCheck=0
		userChoice="o"
	else
		echo "USAGE :
	installUnicap.sh [-f]

	option :
	\t-f :\tAvec l'option force le script s'executera sans poser de questions.
	"
		exit
	fi
fi

#Verifications dossier de destination
if [ -d "$dirInstallUnicap" ]; then
	echo "ATTENTION : Le dossier $dirInstallUnicap existe deja.
Voici son contenu :
"
	ls -la "$dirInstallUnicap"
	echo "Ce script pourrait modifier son contenu !
"
	if [ $userCheck -eq 1 ]; then
		echo "Souhaitez vous continuer ? (o/n)
	"
		read userChoice
		if [ "$userChoice" != "o" ]; then
			exit
		fi
	fi
	#Tests si les sources sont deja presentes
	if [ -s "$dirInstallUnicap/$versionUnicap.tar.gz" -a -s "$dirInstallUnicap/$nomPatch" ]; then
		echo "Les fichiers \"$versionUnicap.tar.gz\" et \"$nomPatch\" sont deja presents dans le dossier de destination \"$dirInstallUnicap\".

Le script utilisera ces fichiers sources pour installer Unicap.
Les fichiers ne seront donc pas telecharges.

"
		if [ $userCheck -eq 1 ]; then
			echo "Si vous souhaitez poursuivre presser la touche (o).
Si vous souhaitez telecharger les sources vous devez supprimer les fichiers \"$versionUnicap.tar.gz\", \"$nomPatch\" et le dossier \"$versionUnicap\" du dossier \"$dirInstallUnicap\". Relancez ensuite ce script.
		"
			read userChoice
			if [ "$userChoice" != "o" ]; then
				exit
			fi
		fi
		downloadSource=0
	fi
else
	echo "Le dossier $dirInstallUnicap n'existe pas et va etre cree.
"
	mkdir "$dirInstallUnicap"
fi

cd "$dirInstallUnicap"
dirCurrentDir=`pwd`

#Recapitulatif
echo "Ce script va executer les commandes suivantes :

"
if [ $downloadSource -eq 1 ]; then
	echo "- Telecharger dans le dossier \"$dirCurrentDir\" le code source des librairies Unicap :
wget http://unicap-imaging.org/downloads/$versionUnicap.tar.gz

"
fi

echo "- Extraire les sources Unicap :
tar -xvzf $versionUnicap.tar.gz

"
if [ $downloadSource -eq 1 ]; then
	echo "- Telecharger dans le dossier \"$dirCurrentDir\" un patch a appliquer pour faire fonctionner Unicap sous CCV :
wget http://kaswy.free.fr/sites/default/files/download/ps3eye/unicap/$nomPatch

	"
fi
echo "- Patcher les sources Unicap :
patch -p0 < $nomPatch

- Compiler et installer les sources Unicap :
cd $versionUnicap
./configure
make
sudo make install

- Installer libpoco-dev
sudo apt-get install libpoco-dev

"
#Confirmation
if [ $userCheck -eq 1 ]; then
	echo "Souhaitez-vous continuer ? (o/n)
"
	read userChoice
	if [ "$userChoice" != "o" ]; then
		exit
	fi
fi

#Installation

if [ $downloadSource -eq 1 ]; then
	wget http://unicap-imaging.org/downloads/$versionUnicap.tar.gz
fi

tar -xvzf $versionUnicap.tar.gz

if [ $downloadSource -eq 1 ]; then
	wget http://kaswy.free.fr/sites/default/files/download/ps3eye/unicap/$nomPatch
fi

patch -p0 < $nomPatch

cd "./$versionUnicap"
./configure
make
sudo make install

sudo apt-get install libpoco-dev

echo "L'installation de unicap pour CCV est terminee
"

