println abuild.getVariable('potato.salad')

abuild.setParameter('VAR1', 'first value')
abuild.setParameter('VAR2', 'second value')
abuild.setParameter('qtest.export',
                    ['VAR1', 'VAR2', 'potato', 'spackle', 'truth1'])

abuild.setParameter('abuild.rules', ['empty'])
