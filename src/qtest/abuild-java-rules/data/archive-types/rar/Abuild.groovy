parameters {
    abuild.rules = 'java'
    java.highLevelArchiveName = 'rar-example.rar'
    def other = resolve('abuild.classpath')
    other << 'file1'
    java.packageHighLevelArchive << ['filestopackage': other]
}
