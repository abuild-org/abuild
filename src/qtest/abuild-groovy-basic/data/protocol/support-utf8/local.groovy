PrintStream p = new PrintStream(
    new FileOutputStream(
        new File(abuild.buildDirectory, "after.interface")))
def support_ascii = (abuild.resolve('FORCE_ASCII') ? "0" :
                         ("π".length() == 2 ? "0" : "1"))
p.println "declare SUPPORT_UTF8 boolean = $support_ascii"
