/* singleton.cls */
/* define a singleton metaclass; a "metaclass" is always a subclass of "class" */
::class singleton subclass class public
::method init                 /* constructor method                  */
   expose singleInstance            /* direct access attribute       */
   singleInstance=.nil              /* initialize attribute          */

::method new                  /* new instance should be created      */
   expose singleInstance            /* direct access attribute       */
   if singleInstance=.nil then      /* no instance created as of yet?*/
      singleInstance=self~new:super /* create new instance, save it  */
   return singleInstance            /* return instance               */
