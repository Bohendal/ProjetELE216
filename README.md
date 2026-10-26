# ProjetELE216

---

## Steganographie
L'application dissimule des messages (texte ou fichier binaire) dans les bits de poids faible (LSB) des pixels d'une image BMP, et les extrait de manière transparente. L'interface est entièrement en ligne de commande. En phase 2, deux machines échangent des messages cachés à travers des images transmises par socket : un canal de communication clandestin.

## Architecture
Le projet est découpé en quatre modules. A est indépendant. B et C dépendent de A et partagent une spécification commune du format de données cachées. D orchestre le tout.
 
 - Module A (image) : encapsulaton BMP, accès pixel et bit, comparaison
 - Module B (encodeur) : dissimulation d'un message dans l'image
 - Module C (décodeur) : extraction d'un message dans l'image
 - Module D (application CLI) : interface en ligne de commande, gestion des fichiers



