@Library('conservify') _

conservifyProperties()

timestamps {
    node () {
        conservifyBuild(name: 'firmware-common', archive: true, distribute: true)

        build job: "weather", wait: false
        build job: "atlas", wait: false
        build job: "sonar", wait: false
        build job: "core", wait: false
        build job: "naturalist", wait: false
        build job: "fona", wait: false
        build job: "example-module", wait: false
    }

    refreshDistribution()
}
