// To get anything signed, build items still have to set
// java.copiedJars or override signdir.
parameters {
    java.sign.alias = 'abuild-testsuite'
    java.sign.storepass = 'keystore-password'
    java.sign.keystore = resolve(keystore.file)
    java.sign.keypass = 'key-password'
}
