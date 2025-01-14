<?php

namespace Bloom\BuildScripts;

require_once __DIR__ . "/TargetDescriptionFiles/Factory.php";

print "Loading AVR8 target description files.\n";
$avr8Tdfs = TargetDescriptionFiles\Factory::loadAvr8Tdfs();
$failedValidationCount = 0;

foreach ($avr8Tdfs as $targetDescriptionFile) {
    $validationFailures = $targetDescriptionFile->validate();

    if (!empty($validationFailures)) {
        $failedValidationCount++;

        print "\033[31m";
        print "Validation for " . $targetDescriptionFile->filePath . " failed.\n";
        print count($validationFailures) . " error(s) found:\n";
        print implode("\n", $validationFailures);
        print "\n\n";
        print "\033[0m";

    } else {
        print "\033[32m";
        print "Validation for " . $targetDescriptionFile->filePath . " passed.\n";
        print "\033[0m";
    }
}

print "\n\n";
print "Validated " . count($avr8Tdfs) . " TDFs. ";
print (($failedValidationCount > 0) ? "\033[31m" : "\033[32m");
print $failedValidationCount . " failure(s)." . "\033[0m" . "\n";
print "Done\n";
