@Library('conservify') _

conservifyProperties()

timestamps {
    node () {
        conservifyBuild(name: 'firmware-common')

        build job: "weather", wait: false
        build job: "atlas", wait: false
        build job: "sonar", wait: false
        build job: "core", wait: false
    }

    refreshDistribution()
}
