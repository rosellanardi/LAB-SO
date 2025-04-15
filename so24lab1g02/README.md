# Compilación

Para compilar el proyecto, simplemente ejecutar: 

```bash
make
```
Se generará un archivo ejecutable con el nombre del proyecto en la carpeta LAB1, el cual se puede ejecutar con el comando: 

```bash
./mybash
```

# Tests

Se provee una test-suite implementada con check a fin de comprobar que la implementación dada
tiene alguna parte de la funcionalidad esperada. 

Para instalar check utilizar el siguiente comando: 

```bash
sudo apt-get install check
```

- Para compilar e invocar el unit testing de scommand y pipeline, basta con hacer:

```bash
make test-command
```

- Para compilar e invocar el unit testing de parsing, basta con hacer:

```bash
make test-parsing
```

- Para compilar e invocar el unit testing general del proyecto, con enfasis en execute, basta con hacer: 

```bash
make test
```

- Para compilar e invocar el unit testing para chequear perdidas/manejo de memoria, basta con hacer: 

```bash
make memtest
```
- Para mas informacion acerca de los test cheaquear el archivo "Makefile" ubicado en la carpeta LAB1

# Observaciones

Los comandos de la forma: 

```bash
cmd | ...
```
Es decir donde figura el pipe pero no ingresamos un segundo comando seguido de este, seran considerados un sintactico.
