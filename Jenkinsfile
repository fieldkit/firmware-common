@Library('conservify') _

conservifyProperties()

timestamps {
    node () {
        sh "rm -rf core/config.h"

        conservifyBuild(name: 'firmware-common', archive: "build/core/*.bin, build/module/*.bin")

        distributeFirmware(module: 'fk-core', directory: "build/core")
        distributeFirmware(module: 'fk-testing-module', directory: "build/module")

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
