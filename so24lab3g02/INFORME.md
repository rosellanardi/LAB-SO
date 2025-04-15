# Laboratorio 3: Planificador de procesos.

## Implementación.

## Primera Parte: Estudiando el planificador de xv6-riscv.

1. **¿Qué política de planificación utiliza `xv6-riscv` para elegir el próximo proceso a ejecutarse?** 

La política de planificación que utiliza `xv6-riscv` para elegir el próximo proceso a ejecutarse es **Round Robin**. En primer lugar, recordemos que **Round Robin** es un algoritmo de planificación que se basa en ejecutar un proceso durante un time slice (generalmente llamado **quantum**), seleccionando el siguiente de la cola de procesos cuando el mismo se acaba. Podemos detectar esta política en `xv6-riscv` debido a principalmente dos razones:

Por un lado, observamos que la función `scheduler` en `kernel\proc.c` realiza un recorrido en un arreglo donde se encuentran todos los procesos disponibles (un máximo de 64). Esto nos da la noción de una cola o lista de de los procesos próximos a ejecutarse.

Por otro lado, pudimos detectar que se invoca a funciones que producen cambios de contexto cada cierto intervalo de tiempo, lo cual nos da la referencia de que existe un **quantum** (detallado próximamente), y no necesariamente por la terminación del proceso.

Gracias a estas características podemos diferenciar el uso de **Round Robin** como política de planificación.

2. **¿Cúales son los estados en los que un proceso puede permanecer en xv6-riscv y qué los hace cambiar de estado?**

Los posibles estados en los que puede permanecer un proceso en `xv6-riscv` son: 

```c
enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
```
`(kernel\proc.h)`

Los procesos en xv6 pueden cambiar de un estado a otro debido a múltiples situaciones, tales como: 

- La espera de un I/O mediante las funciones `sleep` (**Running->Sleeping**) and `wakeup` (**Sleeping->Runnable**).
- Cuando un proceso hijo finaliza mediante `exit` (**Running->Zombie**).
- En el tiempo en que se crea la tabla de procesos seteando el estado de los procesos en unused, y cada vez que se quiere crear un nuevo proceso con la función `allocproc` (**Unused->Used**). 
- Asimismo, cuando el sistema ejecuta un `context-switch`, esto genera un cambio en el estado del proceso que se está ejecutando y, en el caso que haya, también cambia el estado del nuevo proceso a ejecutarse, mediante las funciones `yield`, `sched`, `switch` y `scheduler` (**Running->Runnable**, **Runnable->Running**).

3. **¿Qué es un *quantum*? ¿Dónde se define en el código? ¿Cuánto dura un *quantum* en `xv6-riscv`?**

En relación a la política de planificación **Round Robin** que utiliza `xv6-riscv`, un **quantum** hace referencia al intervalo de tiempo que un proceso tiene disponible para ejecutarse en el CPU antes de que se produzca un `context-switch`. Sin embargo, detectamos que no existe una definición explícita del valor del quantum, sino que este se relaciona intrínsecamente con el `timer interrupt`; es decir, a un proceso se le acaba su **quantum** cuando sucede este mecanismo de hardware.

Con respecto a cuanto dura el **quantum**, este va a tomar el valor de la duración entre las interrupciones, definido a continuación en la función `timerinit`:

```c
int interval = 1000000; // cycles; about 1/10th second in qemu
```
`(kernel\start.c)`

Esto nos da la pauta de que el quantum de un proceso será de 1.000.000 de ciclos de CPU, lo cual es equivalente a pensar en 0.1 segundos o 100ms. Además, nos ayuda a complementar nuestra afirmación de que `xv6-riscv` utiliza **Round Robin** como planificador.

4. **¿En qué parte del código ocurre el cambio de contexto en `xv6-riscv`? ¿En qué funciones un proceso deja de ser ejecutado? ¿En qué funciones se elige el nuevo proceso a ejecutar?**

El **context switch** ocurre cuando se termina el **quantum** del proceso, es decir, cuando ocurre un  `timer interrupt`, o también cuando el proceso en ejecución cede el control del CPU (puede ser por **I/O**, o bien cuando el proceso termina).

El cambio de contexto se produce entre las funciones `yield` y `sched`, complementadas de `scheduler` y `swtch`, todas definidas en `kernel\proc.c` 

- **yield**: es la función que se encarga de desplanificar el proceso en ejecución (**Running->Runnable**) llamando a `sched`.
- **sched**: realiza algunas comprobaciones necesarias y llama a `swtch` para guardar los registros del contexto del CPU en los registros del contexto del proceso.
- **scheduler**: se encarga de ejecutar un bucle infinito recorriendo los procesos disponibles y ejecutando aquel que tome el lock (**Runnable->Running**). Además, invoca a `swtch` para guardar los registros del contexto del proceso en los registros del CPU.
- **swtch**: a partir de un **context old** y un **context new**, guarda en **old** los registros cargados desde **new**. Se utiliza de forma distinta en `sched` y `scheduler` según de qué forma deben cargarse/guardarse los registros.

A partir de esta especificación, podemos notar que mientras en `yield` un proceso deja de ser ejecutado, en `scheduler` se elige el nuevo.

5. **¿El cambio de contexto consume tiempo de un *quantum*?**

Cuando al proceso en ejecución se le termina el **quantum**, este debe pasar de **Running a Runnable**. En ese tiempo, las interrupciones están deshabilitadas por lo que no se consume tiempo del nuevo **quantum**. Una vez que ésta tarea terminó, se habilitan las interrupciones y se empieza a utilizar el tiempo del **quantum** para cargar el nuevo proceso al CPU.

Como conclusión, el cambio de contexto utiliza tiempo del **quantum** solo para la carga del proceso seleccionado por el **scheduler** y no cuando el anterior fue desplanificado. La duración que tendrá disponible el nuevo proceso dependerá de cuan complejo es el cambio de contexto.


## Segunda Parte: Medir operaciones de cómputo y de entrada/salida.

**Experimento 1: ¿Cómo son planificados los programas iobound y cpubound?**

Para realizar este experimento y comprar la planificación de los programas **iobound** y **cpoubound** utilizamos las siguientes métricas:

**iobound**:
```c
metric = (elapsed_ticks * 100000) / total_iops;
```
**cpubound**:
```c
metric = (elapsed_ticks * 100000) / total_cpu_kops;
```
Esta métrica nos permite conocer cuántos ciclos son necesarios para realizar 1000 operaciones en el programa **cpubound** y una operación en el programa **iobound**. Además, nos muestra fácilmente cuantos milisegundos y **quantums** transcurren para el mismo resultado sabiendo que 100.000 ciclos equivalen a 10ms.

| **Experimento** | **Cantidad de ciclos para realizar 1 operación (MEDIA)** | **Tiempo en ms total del experimento (solo IO'S)** |
|:--------------:|:--------------------------------------------------------:|:--------------------------------------------------:|
|      2 IO      |                        10903,4063                        |                      1116,5088                     |
|      3 IO      |                        14762,9167                        |                      1511,7227                     |
|  2 IO + 3 CPU  |                        49131,2656                        |                      5031,0416                     |
|  3 IO + 1 CPU  |                        27915,9792                        |                      2858,5963                     |

|                     |
|:-------------------:|
| ![Gráfico 1](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=1711587965&format=image) |
| ![Gráfico 2](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=656598453&format=image) |
|                     |

| **Experimento** | **Cantidad de ciclos para realizar 1000 operaciones(MEDIA)** | **Tiempo total en ms  del experimento(solo CPU'S)** |
|:--------------:|:------------------------------------------------------------:|:---------------------------------------------------:|
|      1 CPU     |                            23,4688                           |                      40316,0832                     |
|      2 CPU     |                            50,0313                           |                     171893,6064                     |
|      3 CPU     |                            70,5938                           |                     363811,0464                     |
|  3 CPU + 2 IO  |                            71,0952                           |                     366395,5090                     |
|  1 CPU + 3 IO  |                            26,2813                           |                      45147,5712                     |


|                     |
|:-------------------:|
| ![Gráfico 3](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=999134014&format=image) |
| ![Gráfico 4](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=1602472081&format=image) |
|                     |


1. **Describa los parámetros de los programas cpubench e iobench para este experimento (o sea, los define al principio y el valor de N. Tener en cuenta que podrían cambiar en experimentos futuros, pero que si lo hacen los resultados ya no serán comparables).**

En ambos experimentos se utilizó un **N=32**. Los parámetros de cada uno en particular son los siguientes:

- **CPUBENCH**: realiza un cálculo matemático de multiplicación de matrices que utiliza fuertemente el CPU.
    - **CPU_MATRIX_SIZE** = 128. Representa el tamaño de cada matriz a multiplicar.
    - **CPU_EXPERIMENT_LEN** = 256. Cantidad de veces que multiplica las matrices.
    - **MEASURE_PERIOD** = 1000. Nos permite transformar nuestros resultados en k-operaciones (resultado/MEASURE_PERIOD).

- **IOBENCH**:
    - **IO_OPSIZE** = 64. Cantidad de bytes que lee/escribe en cada operación `read/write`.
    - **IO_EXPERIMENT_LEN** = 512. Cantidad de veces que realiza operaciones de `read/write` (1024 operaciones: 512 de escritura y 512 de lectura).

2. **¿Los procesos se ejecutan en paralelo? ¿En promedio, qué proceso o procesos se ejecutan primero? Hacer una observación cualitativa.**

Al usar **Round Robin** como política los procesos no se ejecutan en paralelo y su ejecución se realiza por orden de llegada. A pesar de que notamos que siempre se ejecutan primero los procesos **CPUBENCH**, esto no es así. En realidad, respetando la política utilizada, cualquier proceso puede consumirse primero. Sin embargo, los **IOBENCH**, al utilizar tan poco tiempo en CPU, no llegan a darnos la información de que se ejecutaron por consola (printf).

3. **¿Cambia el rendimiento de los procesos iobound con respecto a la cantidad y tipo de procesos que se estén ejecutando en paralelo? ¿Por qué?**

Los gráficos (1) y (2) nos ayudan a comprender esta noción. A medida que se agregan procesos cpobund, tanto la cantidad de ciclos necesarios para realizar 1000 operaciones, como el tiempo total del experimento, aumenta en gran medida.

Por un lado, si se ejecutan procesos **iobound** vemos que el rendimiento decae a medida que se agregan procesos de éste tipo pero no en gran medida. Esto se debe a que este los procesos no consumen mucho tiempo en CPU y están la mayor parte del tiempo realizando sus operaciones I/O.

Por otro lado, cuando se agregan procesos **cpubound** el rendimiento se ve ampliamente afectado. Por más que los procesos **iobound** consumen poco tiempo en CPU, deberán esperar que finalice el **quantum** de los procesos **cpubound** que se están ejecutando en paralelo. Esta gran espera del CPU genera que el rendimiento disminuya y el tiempo total de la ejecución crezca notablemente.

4. **¿Cambia el rendimiento de los procesos cpubound con respecto a la cantidad y tipo de procesos que se estén ejecutando en paralelo? ¿Por qué?**

Con respecto a los procesos **cpubound** podemos decir que el rendimiento se ve altamente afectado cuando se ejecuta en paralelo junto con otros procesos del mismo tipo. Lo mismo se debe a que estos tienen una alta demanda de tiempo de CPU el cual deben compartir (mononucleo), generando muchos tiempos en espera (en estado **Runnable**).

Además, podemos notar que si un solo proceso **cpubound** tiene un rendimiento X, al ejecutar 3 procesos **cpubound** en paralelo el rendimiento pasa a ser aproximadamente “cantidad de procesos **cpubound** en paralelo * X”. (Notarlo en el gráfico (3) ).

Por otro lado podemos observar que cuando los procesos **cpubound** se ejecutan en paralelo junto con procesos **iobound**, el rendimiento no se ve afectado: el tiempo en CPU que consumen los procesos **iobound** es proporcionalmente muy pequeño al consumido por los proceso **cpubound**.

Los gráficos (3) y (4) nos permiten visualizar estas conclusiones: mientras que nuestra métricas aumentan a medida que se corren más procesos **cpubound**, las mismas se encuentran iguales si hay más procesos **iobound**.

5. **¿Es adecuado comparar la cantidad de operaciones de cpu con la cantidad de operaciones iobound?**

Partiendo de la base de que son tipos de procesos distintos, no sería adecuado comparar sus operaciones: ambas se realizan en forma y contextos muy distintos. Mientras que una operación **cpubound** necesita solamente de la utilización del CPU, las operaciones **iobound** implican tanto al CPU como a otros dispositivos (disco, periféricos, etc). Todo esto último involucra una intercomunicación que genera un contexto más complejo.

Lo anteriormente enunciado nos da la pauta de que no existe un parámetro adecuado que nos permita comparar la cantidad de operaciones de cada tipo, siendo que cada una es notablemente diferente a la otra. Además, ambas operaciones utilizan recursos distintos la mayor parte de su tiempo, lo que impide que exista una métrica para compararlas.

Sin embargo, estas dos formas de operaciones pueden estar presentes en un mismo entorno, sin compararse directamente, pero permitiéndonos deducir conclusiones valiosas.


**Experimento 2: ¿Qué sucede cuando cambiamos el largo del quantum?**

1. **¿Fue necesario modificar las métricas para que los resultados fueran comparables? ¿Por qué?**

Sí, fue necesario modificar el valor de la métrica, ya que esta utilizaba como parámetro el valor del **quantum** (que a su vez representa la cantidad de ciclos de CPU). Es decir, al cambiar el valor de la variable `interval` modificamos ambas cosas.

No obstante, la métrica en su significado siguió siendo la misma: representa la cantidad de ciclos necesarios para realizar 1000 operaciones en **cpubound** o una operación en **iobound**. De esta forma, no nos vimos obligados a modificarla, ya que la misma fue consistente en la cantidad de ciclos.

2. **¿Qué cambios se observan con respecto al experimento anterior? ¿Qué comportamientos se mantienen iguales?**

Pudimos observar algunos cambios a medida que bajamos el **quantum**. Por una parte, el rendimiento del **cpubench** se ve desfavorecido para realizar las 1000 operaciones necesarias, ya que cuenta con una menor cantidad de ciclos de CPU disponibles.

Además, podemos notar que con un **quantum** alto, a los procesos **cpubench** no les afecta la ejecución en paralelos de los iobench. A medida que disminuimos el **quantum**, esto si le afecta: no sólo necesita más ciclos, si no que requiere de más tiempo para terminar la ejecución (gráficos 3, 7 y 11).

|                     |
|:-------------------:|
| ![RR distintos Q](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=1910883123&format=image) |
|                     |

Por el lado de los procesos iobench, podemos notar que tienen un comportamiento similar a medida que disminuimos el **quantum**, generando solo un aumento proporcional con respecto a los anteriores quantums. La razón de esto se debe a que los procesos iobench no deben esperar por los largos **quantum** de los **cpubench** que lo utilizan en su totalidad. Sin embargo, hay un incremento notable en la cantidad de interrupciones y por lo tanto de ticks: de esta forma, nuestra métrica se mantiene igual.

3. **¿Con un quantum más pequeño, se ven beneficiados los procesos iobound o los procesos cpubound?**

Con un **quantum** más pequeño se ven beneficiados los procesos iobound frente a los **cpubound**, ocasionado que el planificador se vuelva más interactivo que batchero. La razón principal de este cambio se basa en que, a medida que disminuimos el **quantum**, los proceso **cpubound** se ven interrumpidos con más frecuencia, generando más contexts switch e imposibilitando una ejecución más continua. Por su parte, los procesos iobunds se ven beneficiados ya que, al momento de necesitar usar el CPU, no deben esperar por los largos intervalos de tiempo de uso del procesador por los cpubounds.

Sin embargo, como mencionamos anteriormente, la constante ejecución de interrupciones / **context switch** incrementa los ticks y genera un peor rendimiento de todos los procesos.

## Cuarta Parte: Implementar MLFQ

2. **Repita las mediciones de la segunda parte para ver las propiedades del nuevo planificador.**

Luego de repetir los experimentos anteriores para la política **MLFQ** llegamos a las siguientes conclusiones:

En primer lugar, analizamos que, debido a la complejidad del planificador **MLFQ**, este tiene un peor desempeño a comparación de Round Robin: la cantidad de `acquire’s` y `release’s` que se realizan, más la obligación de recorrer todos los procesos para elegir el mejor, genera una cantidad significativa del consumo del **quantum**. 

Por otro lado, con respecto a los procesos **cpubound**, observamos que estos se ven perjudicados en su rendimiento en comparación a **Round Robin** y a medida que disminuimos el quantum, si se ejecutan en paralelo con procesos **iobound**: no solo se van reduciendo los ciclos de CPU, si no que también debe lidiar con su baja prioridad (**starvation**). Por su parte, procesos **cpubound** individuales tienen un comportamiento similar a **Round Robin**. En cuanto a la diferencia de comportamiento podemos observar en el próximo gráfico que en **Round Robin** si tenemos tres procesos **cpubound** y le agregamos dos procesos iobounds el rendimiento se mantiene, mientras que en **MLFQ** el rendimiento decae un 50%:

|                     |
|:-------------------:|
| ![Gráfico MLFQ VS RR](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=1424649093&format=image) |
|                     |

Con respecto a los procesos **iobound**, estos se ven beneficiados por la gran cantidad de veces que son seleccionados por el **scheduler** -por su alta prioridad-. Además, a medida que disminuye el **quantum** y se agregan procesos **cpubound** en paralelo, su rendimiento se estabiliza en comparación a ejecutarlos solos.  Podemos observar la diferencia de comportamiento entre **RR** y **MLFQ** orientandonos por el siguiente gráfico:

|                     |
|:-------------------:|
| ![Gráfico MLFQ VS RR](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=1731741744&format=image) |
|                     |

En **RR** al agregarle un proceso **cpubench** a los 3 **iobench** se puede ver que su rendimiento decae un 100% aproximadamente, mientras que en **MLFQ** decae un 50%.

3. **Para análisis responda: ¿Se puede producir starvation en el nuevo planificador? Justifique su respuesta.**

Si, se produce **starvation**. Pudimos notar que a medida que agregamos procesos **iobound** a la ejecución, los **cpubound** van tomando cada vez más tiempo. Esto se da porque los **iobound** se mantienen en alta prioridad por lo que sí tenemos muchos de estos procesos nunca se llegarán a ejecutar los **cpubound**, que tienen mínima prioridad, hasta que se terminen los **iobound** -*o hasta que estén durmiendo todos en simultáneo*-. El siguiente gráfico nos muestra esta situación:

|                     |
|:-------------------:|
| ![Gráfico MLFQ VS RR](https://docs.google.com/spreadsheets/d/e/2PACX-1vQJwozaLXVd8P9QYOp2sX3fE85BrykyxMC5h3osqWAQJFdx7d046xPVakhSY3zTOoKm-_tYKOpy8tgL/pubchart?oid=611608150&format=image) |
|                     |


## Otras tablas y gráficos:

https://docs.google.com/spreadsheets/d/1rTJUHGjUXP2tmWcsd8rSXLZ0AwDA89IjnRk3Zr8YDms/edit?usp=sharing

## Puntos estrella:

- Se reemplazó la política de ascenso de prioridad por la regla 5 de MLFQ de OSTEP: *Priority boost* (rama mlfq-priorityboost)