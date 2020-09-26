SELECT t2.name, SUM(level)
FROM CatchedPokemon AS cp2, Trainer AS t2
WHERE cp2.owner_id = t2.id
GROUP BY t2.id
HAVING SUM(level) >= ALL (
  SELECT SUM(level)
  FROM CatchedPokemon AS cp, Trainer AS t
  WHERE cp.owner_id = t.id
  GROUP BY t.id
  )
ORDER BY t2.id ASC