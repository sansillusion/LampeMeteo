# Lampe Météo pour esp32

Utilise un seul esp32 pour rouler 108 ws2812b disposée en 6 groupes de 18 au tour d'un exagone à l'intérieur de la lampe
[Voir video](https://www.youtube.com/watch?v=d4T23pGf4tQ&ab_channel=SteveOlmstead).

## Configuration

### Flash
Vous devez obtenir une clef API pour openWeather et une pour google Maps,
et les enregistrer dans le fichier .ino :

```markdown
String openWeatherID = "YourKeyHere";
String googleMapsID = "YourKeyHere";

```
### Soudures

Vous devez connecter le circuit de signal des dels sur la pin 18 (n'oubliez pas une r/sistance entre 100 et 250).
Assurez vous d'avoir une source de courrant de 5v d'environs 8amps.

### Démarage

- première fois:

1. Connectez-vous sur le wifi "meteo"
2. Utilisez le gestionnaire de connection (sur android) ou allez à la page [http://192.168.4.1](http://192.168.4.1)
3. Choisisez "configure wifi"
4. Entrez les informations de votre routeur wifi et cliquez "save"
5. La lampe se connecte sur votre wifi
6. Visitez [http://meteo.local](http://meteo.local) sur ordinateurs ou utilisez [Zentri Discovery](https://play.google.com/store/apps/details?id=discovery.ack.me.ackme_discovery&hl=en) sur Android
7. Ayez du plaisir pendents des heures !

## Pour plus d'informations ou d'aide n'hésitez pas !
