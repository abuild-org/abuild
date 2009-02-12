println abuild.getVariable('potato.salad')

abuild.setParameter('VAR1', 'first value')
abuild.setParameter('VAR2', 'second value')
abuild.setParameter('QTest.export',
                    ['VAR1', 'VAR2', 'potato', 'spackle', 'truth1'])

abuild.setParameter('abuild.rules', ['Empty'])
