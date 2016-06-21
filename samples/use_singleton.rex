/* use_singleton.rex */
o1=.singletonExample~new   /* create an instance                     */
say "o1~createtime:" o1~createtime "o1~identityHash:" o1~identityHash

call syssleep .1           /* make sure 1/10 second passes by        */
o2=.singletonExample~new   /* create another instance                */

say "o2~createtime:" o2~createtime "o2~identityHash:" o2~identityHash
say "o1==o2?" (o1==o2)     /* are both objects identical (the same) object?   */

::requires "singleton.cls" /* get access to the public metaclass singleton    */

/* =================================== */
::class singletonExample metaclass singleton /* use the singleton metaclass   */
/* =================================== */
/* instance methods to verify it works */
/* =================================== */
::attribute createtime
/* ----------------------------------- */
::method init              /* constructor method                     */
   expose createTime
   createTime=.dateTime~new
   say "init running at" createTime "for new object:" self~identityHash
