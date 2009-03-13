parameters {
    abuild.rules = 'java'
    java.rarName = 'rar-example.rar'
    def other = resolve('abuild.classpath')
    other << 'file1'
    java.packageRar << ['otherfiles': other]
}
