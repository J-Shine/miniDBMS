SELECT Pokemon.name
FROM Pokemon, Evolution
WHERE Pokemon.id = Evolution.after_id
AND Pokemon.name NOT IN(
  SELECT Pokemon.name
  FROM Pokemon, Evolution
  WHERE Pokemon.id = Evolution.before_id
  )
ORDER BY Pokemon.name