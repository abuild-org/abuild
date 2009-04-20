import org.abuild.QTC

abuild.addTargetClosure('tctest') {
    QTC.TC("dyn", "test coverage case")
}
