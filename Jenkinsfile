@Library('conservify') _

conservifyProperties()

timestamps {
    node () {
        conservifyBuild(name: 'firmware-common')

        build job: "distribution", parameters: []

        build job: "weather", wait: false, parameters: []
        build job: "atlas", wait: false, parameters: []
        build job: "sonar", wait: false, parameters: []
        build job: "core", wait: false, parameters: []
    }
}
