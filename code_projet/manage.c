#include <getopt.h>
#include <stdio.h>

/*
CF GITHUB ENSEIGNANT

Options du programme
    Le programme dispose de plusieurs options :

    -h ou --help : affiche l'aide du programme ainsi que la syntaxe d'exécution du programme
    --dry-run : test l'accès à la liste des processus sur la machine locale et/ou distante sans les afficher
    -c ou --remote-config : spécifie le chemin vers le fichier de configuration contenant les informations de connexion sur les machines distantes
    -t ou --connexion-type : spécifie le type de connexion à utiliser pour la connexion sur les machines distantes (ssh, telnet)
    -P ou --port : spécifie le port à utiliser pour le type de connexion choisi. Si cette option n'est pas spécifié, alor le port par défaut du type de connexion est choisi
    -l ou --login : spécifie l'identifiant de connexion et la machine distante. Ex : --login user@remote_server. remote_server est soit l'IP ou le nom DNS de la machine distante
    -s ou --remote-server : spécifie le nom DNS ou l'IP de la machine distante
    -u ou --username : spécifie le nom de l'utilisateur à utiliser pour la connexion
    -p ou --password : spécifie le mot de passe à utiliser pour la connexion
    -a ou --all : spécifie au programme de collecter à la fois la liste des processus sur la machine local et les machines distantes. S'utilise uniquement si l'option -c ou -s est utilisé.
    NB : Lorsqu'aucune option n'est spécifiée, alors le programme affiche uniquement les processus de la machine locale (la machine sur laquelle le programme est exécuté).

L'option -c ou --remote-config
    Cette option permet de spécifier le chemin vers un fichier au format texte contenant les informations de connexion sur les machines distantes. Pour cela :

    Le format du fichier suit la structure suivante :

    nom_serveur1:adresse_serveur:port:username:password:type_connexion1
    nom_serveur2:adresse_serveur:port:username:password:type_connexion2
    nom_serveur : spécifie le nom à utiliser pour l'affichage des processus de cette machine dans le tableau de bord mutualisé
    adresse_serveur : spécifie l'adresse IP ou le nom DNS de la machine distante
    port : spécifie le port à utiliser pour la connexion sur la machine distante
    username : spécifie le nom d'utilisateur à utiliser pour la connexion
    password : spécifie le mot de passe à utiliser pour la connexion
    type_connexion : spécifie le protocole à utiliser pour la connexion (ssh, telnet)
    Le fichier de configuration doit être caché (renommé comme suit : .nomfichier) et avoir les droits suivants rw------- ou 600. Lors de l'exécution du programme, si le fichier de configuration n'a pas les bons droits, une alerte est levée pour attirer le regard de l'utilisateur.

    Lorsqu'aucun chemin de fichier de config n'est fourni, alors le fichier est cherché dans le répertoire courant sous le nom .config.

L'option -s ou --remote-server
    Cette option permet de spécifier une machine distante via son adresse IP ou son nom de domaine (DNS). Si les options -u et -p ne sont pas specifiés dans la commande, alors il faudra les demandé interactivement lors de l'exécution.

L'option -l ou --login
    Cette option permet de spécifier l'indentifiant et l'adresse de la machine distante suivant la syntaxe suivante -l user@server1.edu ou --login user2@192.168.1.1.

    Si l'option -p n'est pas spécifié dans la commande, alors il faudra demandé interactivement le mot de passe de l'utilisateur.


*/

int main(int argc, char* argv []) {



    //------------- Début récupération des options ------------------//

    // Liste de toute les options, avec : nom_long, argument?, ..., nom_court + description from github enseignant
    struct option options_list[] = {
        {"help",            no_argument,       0, 'h'}, // Afficher l'aide au programme
        {"dry-run",         no_argument,       0, 'd'}, // "Dry run" = "Test à blanc". Test l'accès à la liste des processus sur la machine locale et/ou distante sans les afficher
        {"remote-config",   required_argument, 0, 'c'}, // Spécifie le chemin vers le fichier de configuration contenant les informations de connexion sur les machines distantes
        {"connexion-type",  required_argument, 0, 't'}, // Spécifie le type de connexion utilisé entre les machines (ssh, telnet)
        {"port",            required_argument, 0, 'P'}, // Spécifie le port à utiliser pour le type de connexion choisi. Si cette option n'est pas spécifié, alor le port par défaut du type de connexion est choisi
        {"login",           required_argument, 0, 'l'}, // Spécifie l'identifiant de connexion et la machine distante. Ex : --login user@remote_server. remote_server est soit l'IP ou le nom DNS de la machine distante
        {"remote-server",   required_argument, 0, 's'}, // Spécifie le nom DNS ou l'IP de la machine distante
        {"username",        required_argument, 0, 'u'}, // Spécifie le nom de l'utilisateur à utiliser pour la connexion
        {"password",        required_argument, 0, 'p'}, // Spécifie le mot de passe à utiliser pour la connexion
        {"all",             no_argument,       0, 'a'}, // Spécifie au programme de collecter à la fois la liste des processus sur la machine local et les machines distantes. S'utilise uniquement si l'option -c ou -s est utilisé.
        {0, 0, 0, 0} // Fin de tableau obligatoire
    };

    // ? (github) NB : Lorsqu'aucune option n'est spécifiée, alors le programme affiche uniquement les processus de la machine locale (la machine sur laquelle le programme est exécuté).

    
    // Variable à utiliser plus tard dans le code, correspondant aux options (dans l'ordre de la liste précédente). Les valeurs actuellement affecté sont les valeurs par defaut
    int dry_run = 0;
    char remote_confg[255] = ".confing"; // .config est le fichier .txt de base (attention ,est ce qu'il existe déjà dans le repertoir courant ou pas ? Sinon, le créer dans le repertoire courant)
    int port; // valeur par défaut ?
    char login[255]; // Idem que pour remote_config ?
    char remote_server[255]; // Idem que pour remote_config ?
    char username[255]; // Idem que pour remote_config ?
    char password[255]; // Idem que pour remote_config ?
    int all = 0;


    // Récupération en tant que telle des options    
    int opt = 0;
    while( (opt = getopt_long(argc, argv, "hdc:t:P:l:s:u:p:a", options_list, 0) != -1) ) {
        switch(opt) {
            case 'h':
                print_help(); // Simple fonction avec des printf (ou autre print peut être si cela doit être autre part que le terminal ?). Simplement pour décharger visuellement le main
                break;
            
            case 'd':
                dry_run = 1;
                break;

            case 'c':
                // remote-config --> Affecter le chemin à un char* qu'on utilisera plus tard ?
                break;
            
            case 't':
                //  connexion-type --> Affecter le type de connexion à un char* ?
                break;

            case 'P':
                // port --> Affecter la valeur à un int ?
                break;
            
            case 'l':
                // login --> Affecter login à un char* ?
                break;
            
            case 's':
                // remote-server --> Affecter le nom DNS ou l'IP à un char* ?
                break;

            case 'u':   
                // username --> Affecter le nom d'utilisateur à un char* ?
                break;

            case 'p':
                // password --> Affecter le mot de passe à un char* ?
                break;
            
            case 'a':
                all = 1;
                break;
        }
    }


    //------------------------- Fin récupération des options -------------------------------//

}
