@Library('conservify') _

conservifyProperties()

timestamps {
    node () {
        conservifyBuild(name: 'firmware-common')

        build job: "distribution", parameters: []

        build job: "weather", parameters: []
        build job: "atlas", parameters: []
        build job: "sonar", parameters: []
        build job: "core", parameters: []
    }
}
