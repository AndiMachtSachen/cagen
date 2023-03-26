/*
 * This Kotlin source file was generated by the Gradle 'init' task.
 */
package cagen

import java.nio.file.Paths
import kotlin.io.path.ExperimentalPathApi
import kotlin.io.path.PathWalkOption
import kotlin.io.path.walk
import kotlin.test.Test

class AppTest {
    @OptIn(ExperimentalPathApi::class)
    @Test
    fun compileableExamples() {
        Paths.get("examples").walk(PathWalkOption.BREADTH_FIRST)
            .filter { it.endsWith(".sys") }
            .forEach {
                ParserFacade.loadFile(it.toFile())
            }
    }
}



