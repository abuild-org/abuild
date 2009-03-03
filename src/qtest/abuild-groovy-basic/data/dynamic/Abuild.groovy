parameters {
    abuild.rules = 'empty'
    qtest.export << 'VAR1'
    qtest.export << 'VAR2'
    qtest.export << 'potato'
    qtest.export << 'spackle'
    qtest.export << 'truth1'
    VAR1 = 'first value'
    VAR2 = 'second value'
}

println abuild.resolve('potato.salad')
println abuild.resolveAsString('VAR1')
println abuild.resolveAsList('qtest.export').join(',')
println abuild.resolveAsList('VAR1').join(',')
