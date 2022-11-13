#!/bin/bash
dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/data.csv"
psql postgresql://postgres:bazepodataka@127.0.0.1:5432/VideoGamePublishers -f - <<EOF
\copy (select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers", dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname", games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre from publishers left join directors dirPublisher on "idDirector" = dirPublisher.id right join studios on publishers.id = "idPublisher" join directors dirStudio on studios."idDirector" = dirStudio.id join games on games."idStudio" = studios.id) to '$dir' with (format csv, header)