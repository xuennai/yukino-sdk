plugins {
    id("com.android.library")
    `maven-publish`
}

android {
    namespace = "com.yukinoshita.pdfium"
    compileSdk = 36

    ndkVersion = "29.0.14206865"

    defaultConfig {
        minSdk = 35

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndk {
            abiFilters.add("arm64-v8a")
        }
    }

    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_21
        targetCompatibility = JavaVersion.VERSION_21
    }

    kotlin {
        jvmToolchain(21)
    }

    publishing {
        singleVariant("release")
    }
}

afterEvaluate {
    publishing {
        publications {
            create<MavenPublication>("release") {
                from(components["release"])
                val rawVersion = System.getenv("VERSION") ?: project.version.toString()

                groupId = "com.github.xuennai"
                artifactId = "yukino-sdk"
                version = rawVersion.removePrefix("v")
            }
        }
    }
}

dependencies {
    // ignore
}
