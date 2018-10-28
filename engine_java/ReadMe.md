```java
File f = new File(filePathString);
if(f.exists() && !f.isDirectory()) { 
    // do something
}

//        if (integerThreadLocal.get() == null) {
//            integerThreadLocal.set(0);
//        }
//        int curVal = integerThreadLocal.get();
//        if (curVal % 100000 == 0) {
//            System.out.println(Thread.currentThread().getId() + ", " + curVal);
//        }
//        integerThreadLocal.set(curVal + 1);


   
```