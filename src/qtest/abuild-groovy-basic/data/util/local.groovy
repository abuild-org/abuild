import org.abuild.groovy.Util

// This tests the Groovy version of absToRel using the same test cases
// as in test_util.cc for the C++ version.

assert '../../three' ==
    Util.absToRel("/one/two/three",
                  "/one/two/four/five")
assert '../four/five' ==
    Util.absToRel("/one/two/four/five",
                  "/one/two/three")
assert '..' ==
    Util.absToRel("/one/two/three",
                  "/one/two/three/four")
assert 'four' ==
    Util.absToRel("/one/two/three/four",
                  "/one/two/three")
assert '.' ==
    Util.absToRel("/one/two/three",
                  "/one/two/three")

if (Util.inWindows)
{
    assert '.' ==
        Util.absToRel("/ONE/two/THREE",
                      "/one/TWO/Three")
    assert '../../../D/E/F' ==
        Util.absToRel("C:/A/B/C/D/E/F",
                      "c:/a/b/c/q/r/s")
    assert 'Q:\\w\\w\\w' ==
        Util.absToRel("Q:/w/w/w",
                      "R:/x/x/x")
}

println "assertions passed"
