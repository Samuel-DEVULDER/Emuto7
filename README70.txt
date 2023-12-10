
                    TTTTTTT    OOOOO    7777777
                       T      O     O        7
                       T      O     O       7     77777  000
                       T      O     O      7         7  0   0
                       T      O     O     7         7   0   0
                       T       OOOOO     7         7     000

                             EMULATEUR

                          Par Sylvain HUET
                      (Sylvain.Huet@inria.fr)
                               1996

 README70 : notice d'utilisation

   VERSION 1.5
 ---------------

 Ce programme permet d'emuler le TO7-70 sous Unix et XWindow. La version
ici presentee correspond a un TO7-70 avec stylo optique, manettes de jeu,
et lecteur de k7.
 Le TO7-70 est un ordinateur ouvert : la majeure partie de sa ROM (16ko)
est ammovible et se presente sous forme de cartouches memo7.

 La version 1.5 apporte les ameliorations suivantes :
. emulation correcte de l'opcode non-documente 01
. emulation fine des PIA6821 (octets de direction de de donnees)

I. INSTALLATION
-----------------

I.1 Installation de l'emulateur
--------------------------------

- se placer dans le repertoire Emuto770/src70
- si vous utilisez une station DecAlpha, editez et modifiez le fichier config.h
- compiler le programme avec 'make' (verifiez les chemins d'acces)
- compiler enterkey :
      - chmod +x makekey
      - makekey
- executer enterkey : vous devez alors definir les 58 touches du to7 ainsi
que les 10 touches permettant d'emuler les manettes de jeu. (garder le pointeur
dans la fenetre vide, et suivre les instructions dans la xterm). le fichier
produit s'appelle "keyto7"
- remettre les fichiers "to770" et "keyto7" dans le repertoire Emuto770/

I.2 Installation des outils
----------------------------

- se placer dans le repertoire Emuto770/tools
- compiler par 'maketools'
      - chmod +x maketools
      - maketools
- vous obtenez trois executables : wav2k7, getrom, getmemo7

II. RECUPERATION DES ROMS
---------------------------

Il y a deux types de roms :
- la rom haute du to7 (de &he800 a &h7fff) : 6144 octets
- les cartouches memo7 (de &h0000 a &h3fff) : 16384 octets

Pour faire fonctionner l'emulateur, vous devez disposer de :
- la rom haute, sous forme d'un fichier 'romto770' de 6144 octets, place dans 
le repertoir Emuto7/
- au moins une cartouche, sous forme d'un fichier de 16384 octets. Placer ces 
fichiers dans le repertoire /Emuto770/memo7

Pour recuperer les roms, deux moyens : utiliser un lecteur d'eprom (c'est le
seul moyen pour les cartouches de jeux), ou utiliser le lecteur de k7 de
votre to7-70. La procedure a suivre est alors la suivante (voir chapitre IV 
pour les conseils d'utilisation de l'utilitaire 'wav2k7') - il vous faudra 
une cassette audio, comme au bon vieux temps :
- pour recuperer la rom haute du to7 :
  + ressortir le lecteur de k7 de votre to7, placer la cartouche basic
  + allumer le to7-70 et taper la commande :
    . SAVEM"ROMTO7",&HE800,&HFFFF,0
  + puis digitaliser la k7 de maniere a former un fichier romto770.wav 
    (utiliser un utilitaire standard), echantillonner sur 8bits, mono, a 44kHz.
  + mettre ce fichier dans /Emuto7/tools
  + y lancer 'wav2k7 romto770.wav' ; ceci produit un fichier 'romto770.k7'
  + lancer 'getrom romto770.k7' ; ceci produit un fichier 'romto7.rom'
  + taper 'mv romto7.rom ../romto770'
- pour recuperer une cartouche du to7-70 :
  + ressortir le lecteur de k7 de votre to7-70, placer la cartouche basic
  + allumer le to7 et taper la commande :
    . SAVEM"MEMO7",0,&H3FFF,0
  + puis digitaliser la k7 de maniere a former un fichier memo7.wav 
    (utiliser un utilitaire standard), echantillonner sur 8bits, mono, a 44kHz.
  + mettre ce fichier dans /Emuto7/tools
  + lancer 'wav2k7 memo7.wav' ; ceci produit un fichier 'memo7.k7'
  + lancer 'getmemo7 memo7.k7' ; ceci produit un fichier 'memo7.rom'
  + le fichier 'memo7.rom' contient l'image de la cartouche, vous pouvez
    le deplacer dans le repertoire Emuto770/memo7 et le renommer.



III. UTILISATION
-----------------

-lancer 'to770' avec les options suivantes :
  -k [nom] : specifier le fichier clavier ("keyto7" par defaut)
  -gray    : si vous utilisez une station a niveaux de gris
  -mono    : si vous utilisez une station strictement monochrome


-apres affichage de la fenetre 'TO7-70', le programme lance le moniteur (dans 
la xterm), qui permet de controler l'execution de l'emulateur.

-la rom basic est chargee automatiquement, a condition qu'elle soit presente 
dans le repertoire Emuto770/memo7. si vous souhaitez en charger une
autre, placez-vous dans le bon repertoire et tapez :
  -load [memo7]
-puis lancez l'emulateur par la commande 'go'

-a tout moment, vous pouvez suspendre l'execution de l'emulateur en
utilisant le bouton droit de la souris. vous pouvez alors la reprendre 
au meme point en retapant 'go' dans la xterm

-pendant l'execution, le stylo optique est emule de la maniere suivante :
  - le bouton du milieu (de la souris) enfonce indique que vous deplacez le
    stylo a la surface de l'ecran
  - le bouton de gauche correspond a la pointe du stylo

-vous pouvez faire un 'reset' en revenant au moniteur (bouton droit de la
 souris), et en tapant 'reset' (ce qui se contente de replacer le pc sur
 sa valeur initiale), puis bien-sur 'go'

-vous pouvez manipuler, depuis le moniteur, le lecteur de k7 de la maniere
 suivante :
 - k7 [nom]   : changer de fichier k7
 - seek [num] : positionner la bande sur une valeure particuliere (en hexa)

Attention aux droits des fichiers k7 :
si l'utilisateur n'est pas proprietaire du fichier, il n'aura qu'un droit de
lecture simple.

-vous avez acces aux fonctions dir, cd et pwd.

-vous pouvez ralentir l'emulateur a l'aide de la fonction 'speed', qui 
 permet de regler une boucle de ralentissement


IV. RECUPERATION DE VOS VIEUX PROGRAMMES SUR K7
-------------------------------------------------

Un utilitaire permet de recuperer vos vieux programmes sur k7 : wav2k7,
place' dans Emuto770/tools (celui-ci est aussi utilise' pour recuperer les 
roms)
 Il suffit de proceder ainsi :
. creez un fichier toto.wav de l'enregistrement de votre programme sur
cassette, en 8bits, mono, a 44.1kHz, a l'aide de n'importe quel soft de 
type "enregistreur".
. lancez : wav2k7 toto.wav
. vous obtenez un fichier toto.k7, qui doit marcher si votre cassette est
encore en bon etat.
. en cas de probleme, assurez-vous que le volume d'enregistrement du .wav
etait suffisant, utilisez eventuellement un equaliseur, en poussant 
la bande 4,5kHz - 6,3kHz



V. AVERTISSEMENT
------------------

 Malgre le travail que represente la creation d'un emulateur, celui que je 
vous livre est un freeware. Je ne demande aucune remuneration. Vous pouvez
modifier le source, a la seule condition de le mentionner lors de l'execution
(ajoutez un printf dans la fonction inimonitor() de monitor.c).

 La version msdos est egalement disponible, et presente quelques differences :
affichage plus rapide, emulation du son (buzzer du to7-70 et sortie k7),

N'hesitez pas a me contacter pour echanger de vieux souvenirs ou pour faire
part de tout probleme de compatibilite rencontre :

mail Sylvain.Huet@inria.fr

Certains programmes d'Infogrames ne fonctionnent pas, car utilisant un
loader tres special. J'espere fixer cela bientot.

VI. REMERCIEMENTS
-------------------

 Un grand merci a :
. Pascal Brisset, un vrai hacker avec toujours un lecteur d'eprom sur lui
. David Winter, et sa jolie collection d'ordinateurs thomson
. Eric Mottet et William Hennebois, d'Infogrames, pour leur collaboration





