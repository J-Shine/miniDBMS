SELECT t.name
FROM Trainer as t, (SELECT owner_id, COUNT(*) AS Num
                    FROM CatchedPokemon
                    GROUP BY owner_id) AS j
WHERE j.Num > 2
AND t.id = j.owner_id
ORDER BY j.Num DESC