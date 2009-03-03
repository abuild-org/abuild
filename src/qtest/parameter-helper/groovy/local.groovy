import org.abuild.groovy.BuildFailure

def outer = 'variable defined outside'
def f = { it + 1 }
parameters {
    a.b.c.d = 'some value'
    x.y.z << 3
    x.y.z << 4
    z << 5
    z = 10
    try
    {
        z << 15
    }
    catch (BuildFailure e)
    {
        assert 'parameter z has been previously set as non-list' == e.message
    }
    q = 'r'
    delete z
    p.q.r << a.b.c.d
    def v = 'some other value'
    a.b.c.d = v
    scope = outer
    s.t.u = x.y.z
    w.w.w = n.u.l.l
    something.'with-dashes' = f(12)
    xjc << ['a': 'b']
    xjc << ['c': 'd']
    to.be.deleted = 'can\'t see this'
}
abuild.setParameter('other1', 'other1 value')
abuild.appendParameter('other2', 'other2 value')
abuild.deleteParameter('to.be.deleted')

def results = abuild.params.keySet().sort().collect {
    k -> [k, abuild.resolve(k)] }
assert [
    ['a.b.c.d', 'some other value'],
    ['abuild.localRules', 'local'],
    ['other1', 'other1 value'],
    ['other2', ['other2 value']],
    ['p.q.r', ['some value']],
    ['q', 'r'],
    ['s.t.u', [3, 4]],
    ['scope', 'variable defined outside'],
    ['something.with-dashes', 13],
    ['w.w.w', null],
    ['x.y.z', [3, 4]],
    ['xjc', [['a': 'b'], ['c': 'd']]]
    ] == results

println "all assertions passed"
