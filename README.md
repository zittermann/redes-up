# Programación orientada a redes
## Compilación del proyecto.
Dentro del directorio root se debe ejectutar el comando ```make all``` el cuál creará dos binarios, siendo ```main``` el webserver y ```client``` el cliente. El binario main se ejecutará en ```http://localhost:8080``` 

**Se debe ejecutar ```./main``` primero para asegurarnos de que el webserver está ejectuándose para cuando el ```client``` envíe la request.**

## Client
Sólo envía un request al ```localhost:8080/ping``` con el body "Ping".

## Main 
### Endpoint disponibles
#### ```/ping```
Recibe un body que diga "ping", en caso de que sea correcto retorna "Pong", caso contrario arroja un 400.

#### ```/hit-and-run.jpg```
Retorna ```hit-and-run.jpg```, debe ser visualizado desde un browser.